#ifndef _DB_H_
#define _DB_H_

#include <string>
#include <vector>
#include <map>
#include <regex>

#include "ext.h"

namespace regnetsearch {

  using ext::binwrite;
  using ext::binread;
  using ext::make;

  struct base_sort {
    int qty;
    std::string name;
    base_sort(const std::string& n) : name(n) { qty = 0; }
    };
  
  typedef std::shared_ptr<base_sort> dbsort;
  
  dbsort nosort;
  
  dbsort makesort(const std::string& name) { 
    return ext::make<dbsort> (name);
    }

  template<class T> struct base_podtable {
    dbsort ds;
    std::vector<T> val;
    
    base_podtable(dbsort s) : ds(s) { val.resize(s->qty); }
    void clear() { for(T& d: val) d = 0; }
    T& operator [] (int i) { val.resize(ds->qty); return val[i]; }
    };

  template<class T> using podtable = std::shared_ptr<base_podtable<T>>;

  typedef podtable<double> numtable;
  
  template<class T> void binwrite(FILE *f, const base_podtable<T>& p) {
    binwrite(f, p.val);
    }

  template<class T> void binread(FILE *f, base_podtable<T>& p) {
    binread(f, p.val);
    }
  
  struct base_stringtable {
    dbsort ds;
    std::vector<int> offsets; 
    std::vector<char> contents;
    base_stringtable(dbsort d) : ds(d) {}
    const char* get(int id) { return &(contents[offsets[id]]); }
    virtual ~base_stringtable() {}
    };
  
  typedef std::shared_ptr<base_stringtable> stringtable;
  
  void binwrite(FILE *f, const base_stringtable& s) {
    binwrite(f, s.offsets);
    binwrite(f, s.contents);
    }
  
  void binread(FILE *f, base_stringtable& s) {
    // assuming that sort already set
    ext::binread(f, s.offsets);
    ext::binread(f, s.contents);
    s.ds->qty = s.offsets.size();
    }
  
  struct stringstable : base_stringtable {

    void set(int index, const std::string& s) {
      offsets.resize(ds->qty);
      int res = offsets.size();
      offsets[index] = contents.size();
      for(char ch: s) contents.push_back(ch);
      contents.push_back(0);
      
      // fmap[s] = res;
      // return res;
      }

    stringstable(dbsort d) : base_stringtable(d) {}
    };
  
  struct dictionary : base_stringtable {
    std::map<std::string, int> fmap;

    int add(const std::string& s) {
      ds->qty++;
      int res = offsets.size();
      offsets.push_back(contents.size());
      for(char ch: s) contents.push_back(ch);
      contents.push_back(0);
      fmap[s] = res;
      return res;
      }

    int findoradd(std::string s) {
      if(fmap.count(s)) return fmap[s];
      else return add(s);
      }

    dictionary(dbsort d) : base_stringtable(d) {}
    };  

  struct edgedb {
    std::vector<std::pair<int, int> > edges;
    
    int add(int i, int j) { edges.emplace_back(i,j); }
    };
  
  void binwrite(FILE *f, const edgedb& e) {
    binwrite(f, e.edges);
    }
  
  void binread(FILE *f, edgedb& e) {
    binread(f, e.edges);
    }
  
  numtable regexsearch(stringtable st, const std::regex& r) {
    numtable result = make<numtable> (st->ds);    
    for(int i=0; i<st->ds->qty; i++)
      result->val[i] = std::regex_search(st->get(i), r) ? 1 : 0;
    return result;
    }
  
  numtable icasesearch(stringtable st, const std::string& s) {
    numtable result = make<numtable> (st->ds);    
    for(int i=0; i<st->ds->qty; i++) 
      result->val[i] = ext::isearch(st->get(i), s) ? 1 : 0;
    return result;
    }
  
  numtable picksearch(stringtable st, const std::string& s) {
    using namespace std::regex_constants;    
    if(s[0] == '/') return regexsearch(st, std::regex(s.substr(1), icase | extended));
    else return icasesearch(st, s);
    }

  template<class T> T total(podtable<T> tab) {
    T result = 0;
    for(int i=0; i<tab->ds->qty; i++) result += tab->val[i];
    return result;
    }
  
  }

#endif
