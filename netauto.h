#ifndef _NETAUTO_H_
#define _NETAUTO_H_

#include "db.h"

namespace regnetsearch {

  // automaton state
  
  struct sBase;
  typedef std::shared_ptr<sBase> state;

  struct sBase {
    dbsort ds;
    numtable distr;
    bool filled;
    double lastenergy, energy;
    std::vector<double> eigen;
    virtual void activate() { if(!distr) distr = make<numtable> (ds); }
    virtual void quickprepare() {
      if(!energy) return;
      eigen = distr->val;
      double factor = energy / (lastenergy - energy);
#ifdef NOTHREADS
      auto it = distr->val.begin();
      for(double& d: eigen) *(it++) += (d *= factor);
#else
      ext::parallelize(ds->qty, [&] (int a, int b) {
        for(int i=a; i<b; i++) distr->val[i] += (eigen[i] *= factor);
        return 0;
        });
#endif
      }
    virtual void quickoff() {
      if(eigen.empty()) return;
#ifdef NOTHREADS
      auto it = distr->val.begin();
      for(double& d: eigen) *(it++) -= d;
#else
      ext::parallelize(ds->qty, [&] (int a, int b) {
        for(int i=a; i<b; i++) distr->val[i] -= eigen[i];
        return 0;
        });
#endif
      eigen.clear();
      }
    virtual void run() {}
    virtual void run(int i) {}
    virtual ~sBase() {}
    virtual void display(std::ostream &os) { os << "base"; }
    virtual state getskip() { return NULL; }
    virtual void optimize() {}
    };
  
  namespace queues {
    using std::queue;

    int getclass(double d) { return std::max(std::ilogb(d) + 21, 0); }
    
    #define MAXQ 64
    
    queue<std::pair<sBase*, int> > aqs[MAXQ];
  
    void add(state s, int i, double v) {
      double& d = s->distr->val[i];
      int c0 = getclass(d);
      d += v;
      int c1 = getclass(d);
      if(c1 != c0) aqs[c1].emplace(&*s, i);
      }
    
    void run() {
      int c = MAXQ-1;
      while(c) 
        if(!aqs[c].empty()) {
          auto t = aqs[c].front();
          aqs[c].pop();
          t.first->run(t.second);
          c++;
          }
        else {
          c--;
          }
      }
    };
  
  void optimizeSkip(state& s) { if(s->getskip()) s = s->getskip(); }

  std::ostream& operator << (std::ostream& os, sBase& b) { b.display(os); return os;}
  
  state nowhere;

  std::deque<state> allstates;
  
  void register_state(state s) { allstates.push_back(s); }
  state& reserve_state() { allstates.push_back(NULL); return allstates.back(); }
  
  std::ostream& operator << (std::ostream& os, state s) {
    if(s == nowhere) return os << "nowhere";
    for(int i=0; i<(int) allstates.size(); i++)
      if(allstates[i] == s)
        return os << "#" << i;
    return os << "(?)";
    }

  struct sSkip : sBase {
    state next;
    void activate() { next->activate(); distr = next->distr; ds = next->ds; }
    sSkip(state _next) : next(_next) { }
    virtual void run() {} 
    virtual void display(std::ostream &os) { os << "skip to " << next; }
    virtual state getskip() { optimizeSkip(next); return next; }
    };

  typedef std::shared_ptr<sSkip> sskip;
  
  sskip noleave = std::make_shared<sSkip> (nowhere);

  sskip register_skip() { sskip s = std::make_shared<sSkip> (nowhere); allstates.push_back(s); return s; }
  
  struct sConditional : sBase {
    numtable c;
    state yes, no;
    sConditional(numtable _c, state _yes, state _no) : c(_c), yes(_yes), no(_no) { ds = c->ds; }
    virtual void display(std::ostream &os) { os << c << " ? " << yes << ":" << no; }
    void run() {
      lastenergy = energy;
      auto& dyes = yes->distr->val;
      auto& dno = no->distr->val;
      auto& co = c->val;
      auto& dh = distr->val;
      energy = ext::parallelize(ds->qty, [&] (int a, int b) {
        double en = 0;      
        for(int i=a; i<b; i++) {
          double& d = dh[i];
          en += fabs(d);
          dyes[i] += co[i] * d;
          dno[i] += (1-co[i]) * d;
          d = 0;
          }
        return en;
        });
      filled = false;
      yes->filled = no->filled = true;
      }
    void run(int i) {
      double cv = c->val[i];
      double &d = distr->val[i];
      queues::add(yes, i, cv*d);
      queues::add(no, i, (1-cv)*d);
      d = 0;
      }
    virtual void optimize() { optimizeSkip(yes); optimizeSkip(no); }
    };
  
  struct sRandom : sBase {
    state yes, no;
    double chance;
    sRandom(double ch, state _yes, state _no) : chance(ch), yes(_yes), no(_no) { }
    virtual void display(std::ostream &os) { os << chance << " ? " << yes << ":" << no; }
    void run() {
      lastenergy = energy;
      auto& dyes = yes->distr->val;
      auto& dno = no->distr->val;
      auto& dh = distr->val;
      energy = ext::parallelize(ds->qty, [&] (int a, int b) {
        double en = 0;
        for(int i=a; i<b; i++) {
          double& d = dh[i];
          en += fabs(d);
          dyes[i] += chance * d;
          dno[i] += (1-chance) * d;
          d = 0;
          }
        return en;
        });
      filled = false;
      yes->filled = no->filled = true;
      }
    void run(int i) {
      double &d = distr->val[i];
      queues::add(yes, i, chance*d);
      queues::add(no, i, (1-chance)*d);
      d = 0;
      }
    virtual void optimize() { optimizeSkip(yes); optimizeSkip(no); }
    };
  
  struct sAccept : sBase {
    virtual void run() {} 
    virtual void display(std::ostream &os) { os << "accept"; }
    };

  struct sStart : sBase {
    state startstate;
    sStart(numtable di, state _next) { distr = di; startstate = _next; }
    virtual void activate() {} // d already set!
    virtual void run() {} 
    virtual void display(std::ostream &os) { os << "start in " << startstate; }
    virtual void optimize() { optimizeSkip(startstate); }
    };

  struct sReject : sBase {
    virtual void run() { } 
    virtual void display(std::ostream &os) { os << "reject"; }
    };
  
  }

#endif
