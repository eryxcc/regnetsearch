#ifndef _NETREGEX_H_
#define _NETREGEX_H_

#include "netauto.h"

namespace regnetsearch {

  // exceptions
  struct wrong_sort : std::exception { dbsort d1; dbsort d2; 
    wrong_sort(dbsort _d1, dbsort _d2) : d1(_d1), d2(_d2) {} };

  dbsort unify(dbsort d1, dbsort d2) {
    if(!d1) return d2;
    if(!d2) return d1;
    if(d1==d2) return d1;
    throw wrong_sort(d1,d2);
    }  
  
  // compilation result
  struct stateinfo {
    state start;
    sskip finish;
    dbsort ds;
    };
      
  struct xBase {  
    virtual ~xBase() {}
    virtual stateinfo compile(dbsort d) = 0;
    };
  
  typedef std::shared_ptr<xBase> expression;
  
  // concatenation
  
  struct xConcat : xBase {
    expression e1, e2;
    xConcat(expression _e1, expression _e2) : e1(_e1), e2(_e2) { }

    stateinfo compile(dbsort ds) {
      auto a1 = e1->compile(ds);
      auto a2 = e2->compile(a1.ds);
      a1.finish->next = a2.start;
      return stateinfo {a1.start, a2.finish, a2.ds};
      }
    };
  
  expression operator >> (expression e1, expression e2) {
    return std::make_shared<xConcat> (e1,e2);
    };
  
  // random choice
  
  struct xRandom : xBase {
    double p;
    expression e1, e2;
    xRandom(double _p, expression _e1, expression _e2) : p(_p), e1(_e1), e2(_e2) {}

    stateinfo compile(dbsort ds) {
      state& s = reserve_state();
      auto a1 = e1->compile(ds);
      auto a2 = e2->compile(ds);
      
      s = std::make_shared<sRandom> (p, a1.start, a2.start);
      s->ds = ds;

      auto finish = register_skip();
      a1.finish->next = finish;
      a2.finish->next = finish;
      
      return stateinfo {s, finish, unify(a1.ds, a2.ds) };
      }
    };
  
  expression operator | (expression e1, expression e2) {
    return std::make_shared<xRandom> (.5, e1,e2);
    };
  
  // conditional
  
  struct xConditional : xBase {
    numtable c;
    expression e1, e0;
    xConditional(numtable _c, expression _eyes, expression _eno) :
      c(_c), e1(_eyes), e0(_eno) { }

    stateinfo compile(dbsort ds) {
      c->lazy();
      if(c->ds != ds) throw wrong_sort(c->ds, ds);
      
      state& s = reserve_state();
      auto a1 = e1->compile(ds);
      auto a0 = e0->compile(ds);
      
      s = std::make_shared<sConditional> (c, a1.start, a0.start);
      s->ds = ds;
      
      auto finish = register_skip();
      a1.finish->next = finish;
      a0.finish->next = finish;
      
      return stateinfo {s, finish, unify(a1.ds, a0.ds) };
      }
    };

  expression cond(numtable c, expression ey, expression en) { 
    return std::make_shared<xConditional>(c, ey, en);
    }
  
  // while

  struct xWhile : xBase {
    numtable c;
    expression e;
    xWhile(expression _e, numtable _c) : e(_e), c(_c) {}

    stateinfo compile(dbsort ds) {
      c->lazy();
      state& s1 = reserve_state();
      auto a1 = e->compile(ds);      
      if(a1.ds && a1.ds != ds) throw wrong_sort(a1.ds, ds);
            
      auto outstate = register_skip();
      
      s1 = std::make_shared<sConditional> (c, a1.start, outstate);
      s1->ds = ds;
      a1.finish->next = s1;
      
      return stateinfo { s1, outstate, ds };
      }
    };  
  
  struct xWhileF : xBase {
    double c;
    expression e;
    xWhileF(expression _e, double _c) : e(_e), c(_c) {}

    stateinfo compile(dbsort ds) {
      state& s1 = reserve_state();
      auto a1 = e->compile(ds);      
      if(a1.ds && a1.ds != ds) throw wrong_sort(a1.ds, ds);
            
      auto outstate = register_skip();
      
      s1 = std::make_shared<sRandom> (c, a1.start, outstate);
      s1->ds = ds;
      a1.finish->next = s1;
      
      return stateinfo { s1, outstate, ds };
      }
    };  
  
  // star expression
  
  struct xStar : xBase {
    expression e;
    double out;
    xStar(expression _e, double o) : e(_e), out(o) {}

    stateinfo compile(dbsort ds) {
      state& s1 = reserve_state();
      auto a1 = e->compile(ds);      
      if(a1.ds && a1.ds != ds) throw wrong_sort(a1.ds, ds);
            
      auto outstate = register_skip();
      
      s1 = std::make_shared<sRandom> (out, outstate, a1.start);
      s1->ds = ds;
      a1.finish->next = s1;
      
      return stateinfo { s1, outstate, ds };
      }
    };
  
  expression operator * (expression e, double x) {
    return std::make_shared<xStar> (e, 1/x);
    }
  
  // no-op

  struct xSkip : xBase {
    stateinfo compile(dbsort d) { 
      auto finish = register_skip();
      return stateinfo {finish, finish, d};
      }
    };

  struct xAccept : xBase {
    stateinfo compile(dbsort ds) {
      state& s = reserve_state();
      s = std::make_shared<sAccept> ();
      s->ds = ds;
      return stateinfo {s, noleave, nosort};
      }
    
    };

  struct xStart : xBase {
    numtable di;
    xStart(numtable d) : di(d) { }
    stateinfo compile(dbsort ds) {
      di->lazy();
      if(ds != nosort) throw wrong_sort(ds, nosort);
      state& s = reserve_state();

      auto outstate = register_skip();
      s = std::make_shared<sStart> (di, outstate);
      return stateinfo {s, outstate, di->ds};
      }
    
    };

  struct xReject : xBase {
    stateinfo compile(dbsort d) { 
      state& s = reserve_state();
      s = std::make_shared<sReject> ();
      s->ds = d;
      return stateinfo {s, noleave, nosort};
      }
    };
  
  expression skip = std::make_shared<xSkip> ();
  expression accept = std::make_shared<xAccept> ();
  expression reject = std::make_shared<xReject> ();

  }

#endif
