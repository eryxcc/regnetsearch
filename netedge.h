#ifndef _NETEDGE_H_
#define _NETEDGE_H_

#include "netregex.h"

namespace regnetsearch {

  using namespace ext;

  struct xEdge;

  typedef std::shared_ptr<xEdge> relation;
  
  struct sEdge : sBase {
    std::shared_ptr<xEdge> e;
    state next;
    sEdge(std::shared_ptr<xEdge> _e, state _next) : e(_e), next(_next) {}
#ifndef NOEDGECACHE
    std::vector<std::pair<double*, double*> > edgecache;
#endif
    virtual void display(std::ostream &os);
    void run();
    void run(int i);
    virtual void optimize() { optimizeSkip(next); }
    };

  struct xEdge : xBase, std::enable_shared_from_this<xEdge> {
    dbsort sortfrom, sortto;
    std::string name;
    relation reversed;
    
    std::vector<int> starts;
    std::vector<int> data;
    
    xEdge(dbsort _a, dbsort _b, const std::string& n) : sortfrom(_a), sortto(_b), name(n) {
      data.resize(sortfrom->qty);
      }

    stateinfo compile(dbsort ds) {
      if(ds != sortfrom) throw wrong_sort(sortfrom, ds);
      state& s = reserve_state();
      auto finish = register_skip();
      s = std::make_shared<sEdge> (shared_from_this(), finish);
      s->ds = sortfrom;
      return stateinfo {s, finish, sortto};
      }
    };
  
  relation makeedge(dbsort s1, dbsort s2, const std::string& name) { 
    return std::make_shared<xEdge>(s1, s2, name);
    }

  void sEdge::display(std::ostream &os) { os << "edge " << e->name << " -> " << next; }

  void sEdge::run() {
    lastenergy = energy;
    auto& dn = next->distr->val;
    auto& dh = distr->val;

    if(ext::threads == 0) {
      auto& est = e->starts;
      auto& edt = e->data;
      auto p = edt.begin();
      for(int i=0; i<ds->qty; i++) {
         int st0 = est[i];
         int st1 = est[i+1];
         if(st0 != st1) {
           double&d = dh[i];
           energy += fabs(d);
           d /= st1-st0;
           for(int st=st0; st<st1; st++) {
             int v = *(p++);
             dn[v] += d;
             }
           d = 0;
           }
         }
      }
    else {
      energy = ext::parallelize(ds->qty, [&] (int a, int b) {
        auto& est = e->starts;
        double en = 0;
        for(int i=a; i<b; i++) {
          int d = est[i+1] - est[i];
          if(d) en += dh[i], dh[i] /= d;
          }
        return en;
        });
#ifndef NOEDGECACHE
      if(edgecache.size() == 0) {
        auto& rest = e->reversed->starts;
        auto& redt = e->reversed->data;
        edgecache.resize(redt.size());
        printf("Caching an edge\n");
        ext::parallelize(next->ds->qty, [&] (int a, int b) {
          for(int i=a; i<b; i++) {
            int st0 = rest[i];
            int st1 = rest[i+1];
            for(; st0<st1; st0++) 
              edgecache[st0].first = &dn[i], 
              edgecache[st0].second = &dh[redt[st0]];
            }
          return 0;
          });
        }
      ext::parallelize(edgecache.size(), [&] (int a, int b) {
        for(int i=a; i<b; i++) *edgecache[i].first += *edgecache[i].second;
        return 0;
        });
#else
      ext::parallelize(next->ds->qty, [&] (int a, int b) {
        auto& rest = e->reversed->starts;
        auto& redt = e->reversed->data;
        for(int i=a; i<b; i++) {
          int st0 = rest[i];
          int st1 = rest[i+1];
          for(; st0<st1; st0++) 
            dn[i] += dh[redt[st0]];
          }
        return 0;
        });
#endif
      ext::parallelize(ds->qty, [&] (int a, int b) {
        for(int i=a; i<b; i++) dh[i] = 0;
        return 0;
        });
      }
    filled = false;
    next->filled = true;
    }

  void sEdge::run(int i) {
    auto& est = e->starts;
    int st0 = est[i];
    int st1 = est[i+1];
    if(st0 != st1) {
      double &d = distr->val[i];
      d /= st1-st0;
      for(int st=st0; st<st1; st++) {
        queues::add(next, e->data[st], d);
        }
      d = 0;
      }
    }
  
  int indent = 0;
  
  void printIndent() { for(int i=0; i<indent; i++) printf(" "); }
  
  template<class T, class... Ts> void present1(dbsort ds, int id, T t, Ts... ts) {
    t(ds, id);
    present1(ds, id, ts...);
    }

  void present1(dbsort ds, int id) {}
  
  auto viewTable(stringtable s, std::string title = "") {
    return [s, title] (dbsort ds, int id) {
      printIndent();
      if(ds != s->ds) printf("bad sort in viewTable\n");
      else printf("%s%s\n", title.c_str(), s->get(id));
      };
    }

  template<class T> auto viewTable(podtable<T> s, std::string title = "") {
    return [s, title] (dbsort ds, int id) {
      printIndent();
      if(ds != s->ds) printf("bad sort in viewTable\n");
      else std::cout << title << s->val[id] << "\n";
      };
    }
  
  template<class... Ts> auto viewEdge(relation r, std::string title, Ts... ts) {
    return [r, title, ts...] (dbsort ds, int id) {
      if(title != "") {
        printIndent(); printf("%s\n", title.c_str());
        indent += 2;
        }
      if(ds != r->sortfrom) { printIndent(); printf("bad sort in viewEdge\n"); }
      else for(int i=r->starts[id]; i<r->starts[id+1]; i++) {
        int id2 = r->data[i];
        present1(r->sortto, id2, ts...);
        }
      if(title != "") indent -= 2;
      };
    }
  
  template<class... T> void present(numtable c, T... t) {
    dbsort ds = c->ds;
    std::vector<int> order;
    for(int i=0; i<ds->qty; i++) 
      if(c->val[i]) order.push_back(i);
    sort(order.begin(), order.end(), [c] (int i, int j) {
      return c->val[i] > c->val[j]; });
    for(int a=0; a<200 && a<(int) order.size(); a++) {    
      int o = order[a];
      printIndent();
      printf("#### %3d. %8.6lf #%d\n", a+1, c->val[o], o);
      indent += 2;
      present1(ds, o, t...);
      indent -= 2;
      }
    }  

  void readEdges(dbsort s1, dbsort s2, 
    relation& straight, const std::string& sname, 
    relation& back,    const std::string& bname, 
    edgedb& db) {
    straight = makeedge(s1, s2, sname);
    back = makeedge(s2, s1, bname);
    
    int e = db.edges.size();

    straight->starts.resize(s1->qty+1);
    straight->data.resize(e);
    straight->reversed = back;
    back->starts.resize(s2->qty+1);
    back->data.resize(e);
    back->reversed = straight;
    
    for(auto& p: db.edges) {
      straight->starts[p.first]++,
      back->starts[p.second]++;
      }
    
    int cum;
    
    cum = 0;
    for(int& i: straight->starts) {
      cum += i;
      i = cum;
      }

    cum = 0;
    for(int& i: back->starts) {
      cum += i;
      i = cum;
      }

    for(auto& p: db.edges) 
      straight->data[--straight->starts[p.first]] = p.second,
      back->data[--back->starts[p.second]] = p.first;    
    }
  
  }

#endif
