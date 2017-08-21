#include <cstdio>
#include <algorithm>
#include <memory>
#include <iostream>
#include <deque>
#include <cmath>
#include <queue>

#include "dblp-edge.h"

using namespace dblp;

#include <sys/time.h>
long long getVa() {
  struct timeval tval;
  gettimeofday(&tval, NULL);
  return tval.tv_sec * 1000000 + tval.tv_usec;
  }

int main() {
  initSorts();
  printf("Reading the database...\n");
  readDatabases();
  printf("Building the relations...\n");
  buildRelations();
  
  numtable c, d;
  
  while(true) {  
  
    int af, pfound;
    
    printf("Note: search is case-insensitive\n");
    printf("Precede the name with '/' for regex search (extended POSIX, case insensitive, slower than simple search)\n");
    
    do {
    #ifdef TEST
      std::string who = "Szymon Torunczyk";
      printf("Looking for the author...\n");
    #else
      std::string who;
      std::cout << "Center at: "; std::getline(std::cin, who);
    #endif
      d = picksearch(authorNames, who);
      printf("Authors found: %d\n", af = total(d));
    } while(!af);
    
    do {
      pfound = 0;
      #ifdef TEST
      printf("Looking for the paper...\n");
      std::string topic = "yperbolic";
      #else
      std::string topic;
      std::cout << "Search for topic: "; std::getline(std::cin, topic);
      #endif
      c = picksearch(paperTitles, topic);
      printf("papers found = %d\n", pfound = total(c));
    } while(pfound == 0);
    
  numtable result = make<numtable>(sPaper);

  // expression x = (cond(c, accept(result), papers >> authors) * 10) >> reject;
  expression x = rPapers >> (cond(c, accept(result), rAuthors >> rPapers)) * 10 >> reject;
  
  allstates.clear();
  auto p = x->compile(sAuthor);
  if(p.finish != noleave) {
    printf("Error: leaves\n");
    exit(1);
    }
  
  for(state s: allstates) s->optimize();
  
  auto st = allstates.begin();
  for(state s: allstates) if(!s->getskip()) *(st++) = s;
  while(allstates.end() != st) allstates.pop_back();

  for(state s: allstates) s->activate();

  printf("states in the automaton = %d\n", (int) allstates.size());
  for(state s: allstates) {
    if(&*s) std::cout << s << ": " << *s << ", [" << s->ds->name << "]\n";
    }
  std::cout << "start state = " << p.start << "\n";

    int precint;
#ifdef TEST
    precint = 4;
#else
    std::cout << "Precision (0=quick, 4=nice): "; std::cin >> precint;
    std::string junk; std::getline(std::cin, junk);
#endif
    double prec = pow(.1, precint);
  
    result->clear();
    int N = allstates.size();
    int ms[N];
    for(int i=0; i<N; i++) ms[i] = 0;
    
    for(int i=0; i<d->ds->qty; i++)
      queues::add(p.start, i, d->val[i]);
    queues::run();
  
    long long zero = getVa();
    
    for(state &s: allstates) s->filled = true;
  
    for(int it=0; it<100; it++) {
      if(it % 10 == 9) {
        printf("(Quick step)\n");
        for(state& s: allstates) if(s->filled) s->quickprepare();
        for(state& s: allstates) if(s->filled) s->run();
        for(state& s: allstates) if(s->filled) s->quickoff();
        }
      double energy = 0;
      int i = 0;
      for(state& s: allstates) if(s->filled) {
        auto t1 = getVa();
        s->run();
        energy += s->energy;
        auto t2 = getVa();
        ms[i++] += t2-t1;
        }
      std::cout << "Iteration #" << it << ": " << energy << " | " << getVa()-zero << "\n";
      if(energy < prec) break;
      }
  
    for(int i=0; i<N; i++) 
      std::cout << allstates[i] << ": " << ms[i] << "\n";
    
    present(result, paperTitles, rAuthors, authorNames);

#ifdef TEST
    break;
#endif
    }
  
  return 0;
  }
