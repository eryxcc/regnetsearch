// extensions of the standard library

#ifndef _EXT_H_
#define _EXT_H_

#include <memory>
#include <cctype>

namespace ext {

template<class T> struct maker {  };

  template<class T> struct maker<std::shared_ptr<T>> { 
    template<class... U> static std::shared_ptr<T> make(U... u) { return std::make_shared<T> (u...); } };
  
  template<class T, class... U> auto make(U... u) { return maker<T>::make(u...); }
  
  int scanerr;
  
  template<class T> void binwrite(FILE *f, const T& x) {
    fwrite(&x, sizeof(x), 1, f);
    }
  
  template<class T> void binread(FILE *f, T& x) {
    scanerr=fread(&x, sizeof(x), 1, f);
    }
  
  template<class T> void binwrite(FILE *f, const std::vector<T>& x) {
    int siz = x.size();
    binwrite(f, siz);
    fwrite(x.data(), sizeof(x[0]), siz, f);
    }
  
  template<class T> void binread(FILE *f, std::vector<T>& x) {
    int siz;
    binread(f, siz);
    x.resize(siz);
    scanerr=fread(x.data(), sizeof(x[0]), siz, f);
    }
  
  template<class T> void binwrite(FILE *f, const std::shared_ptr<T>& what) {
    binwrite(f, *what);
    }

  template<class T, class... U> void binread(FILE *f, std::shared_ptr<T>& what, U... params) {
    what = make<std::shared_ptr<T>> (params...);
    binread(f, *what);
    }

  template<class T, class... U> void binwrite(const char *fname, const T& what, U... params) {
    FILE *f = fopen(fname, "wb");
    binwrite(f, what, params...);
    fclose(f);
    }
  
  template<class T, class... U> void binread(const char *fname, T& what, U... params) {
    FILE *f = fopen(fname, "rb");
    binread(f, what, params...);
    fclose(f);
    }
  
  bool isearch(const char * haystack, const std::string &needle) {
    auto hayend = haystack; while(*hayend) hayend++;
    return std::search(haystack, hayend, needle.begin(), needle.end(),
      [] (char c1, char c2) { return toupper(c1) == toupper(c2); }) != hayend;
    }
  
  bool among(char c) { return false; }
  template<class... T> bool among(char c, char x, T... t) { return (c==x) || among(c,t...); }    
  }

#endif
