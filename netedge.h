#ifndef _NETEDGE_H_
#define _NETEDGE_H_

#include "netregex.h"

namespace regnetsearch {

  using namespace ext;

  struct xEdge;

  struct sEdge : sBase {
    std::shared_ptr<xEdge> e;
    state next;
    sEdge(std::shared_ptr<xEdge> _e, state _next) : e(_e), next(_next) {}
    virtual void display(std::ostream &os);
    void run();
    void run(int i);
    virtual void optimize() { optimizeSkip(next); }
    };

  struct xEdge : xBase, std::enable_shared_from_this<xEdge> {
    dbsort sortfrom, sortto;
    std::string name;
    
    std::vector<int> starts;
    std::vector<int> data;
    
    xEdge(dbsort _a, dbsort _b, const std::string& n) : sortfrom(_a), sortto(_b), name(n) {
      data.resize(sortfrom->qty);
      }

    stateinfo compile(dbsort ds) {
      if(ds != sortfrom) throw wrong_sort();
      state& s = reserve_state();
      auto finish = register_skip();
      s = std::make_shared<sEdge> (shared_from_this(), finish);
      s->ds = sortfrom;
      return stateinfo {s, finish, sortto};
      }
    };
  
  typedef std::shared_ptr<xEdge> relation;
  
  relation makeedge(dbsort s1, dbsort s2, const std::string& name) { 
    return std::make_shared<xEdge>(s1, s2, name);
    }

  void sEdge::display(std::ostream &os) { os << "edge " << e->name << " -> " << next; }

  void sEdge::run() {
    lastenergy = energy; energy = 0;
    auto& dn = next->distr->val;
    auto& dh = distr->val;
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

  void present(numtable c, stringtable s, relation r, stringtable s2) {
    dbsort ds = c->ds;
    std::vector<int> order;
    for(int i=0; i<ds->qty; i++) 
      if(c->val[i]) order.push_back(i);
    sort(order.begin(), order.end(), [c] (int i, int j) {
      return c->val[i] > c->val[j]; });
    for(int a=0; a<40 && a<(int) order.size(); a++) {    
      int o = order[a];
      printf("%3d. %8.6lf %s", a+1, c->val[o], s->get(o));
      if(r && r->sortfrom == ds) {
        printf(":");
        for(int i=r->starts[o]; i<r->starts[o+1]; i++) {
          int o2 = r->data[i];
          printf(" %s", s2->get(o2));
          }
        }
      printf("\n");
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
    back->starts.resize(s2->qty+1);
    back->data.resize(e);
    
    for(auto& p: db.edges) 
      straight->starts[p.first]++,
      back->starts[p.second]++;
    
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
