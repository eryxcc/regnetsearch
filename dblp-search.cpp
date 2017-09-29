#include <cstdio>
#include <algorithm>
#include <memory>
#include <iostream>
#include <deque>
#include <cmath>
#include <queue>

#include "dblp.h"
#include "netedge.h"
#include "parser.h"

#define QLEVELS 21

using namespace dblp;

static relation rAuthors, rPapers, rWhichjournal, rJPapers, rWhichproc, rPPapers;

void buildRelations() {
  readEdges(sPaper, sAuthor, rAuthors, "authors", rPapers, "papers", paperAuthor);
  readEdges(sPaper, sJournal, rWhichjournal, "whichjournal", rJPapers, "jpapers", paperJournal);
  readEdges(sPaper, sProcs, rWhichproc, "whichproc", rPPapers, "ppapers", paperProceedings);
  }

struct dblpparser: parser {

  dblpparser(std::string _s): parser(_s) {}
  
  relation getrelation(std::string s) {
    static auto allRelations = { rAuthors, rPapers, rWhichjournal, rJPapers, rWhichproc, rPPapers };
    for(relation x: allRelations) if(x->name == s) return x;
    return nullptr;
    }

  virtual bool namedNumtable(std::string s, xnumtable& d) { 
    if(s == "paperYear") { 
      auto res = make<numtable> (sPaper);    
      for(int i=0; i<sPaper->qty; i++)
        res->val[i] = paperyears->val[i];
      d = res;
      return true;
      }
    if(s == "paperType") { 
      auto res = make<numtable> (sPaper);    
      for(int i=0; i<sPaper->qty; i++)
        res->val[i] = papertypes->val[i];
      d = res;
      return true;
      }
    return false;
    }
  
  stringtable getstringtable(std::string s) {
    auto namedTables = {
      make_pair("authorName", authorNames),
      make_pair("journalName", journalNames),
      make_pair("paperTitle", paperTitles),
      make_pair("proceedingsCode", proceedingsCodes)
      };
    for(auto x: namedTables) if(x.first == s) return x.second;
    return nullptr;
    }
  };

int main() {

  initSorts();
  printf("Reading the database...\n");
  readDatabases();
  printf("Building the relations...\n");
  buildRelations();
  
  std::string s;
  
  printf("Parsing the expression...\n");
  int ch;
  while((ch = getchar()) != -1)
    s += ch;
    
  dblpparser parser(s);
  
  expression x;
  
  try {
    x = parser.parseexp();
    parser.checkend();
    }
  catch(parseerror err) {
    std::cout << parser.display(err) << "\n";
    exit(1);
    }
  
  printf("Compiling the automaton...\n");

  allstates.clear();
  auto p = x->compile(nosort);
  if(p.finish != noleave) {
    printf("Error: leaves\n");
    exit(1);
    }
  
  printf("Optimizing the automaton...\n");

    for(state s: allstates) {
      auto ss = std::dynamic_pointer_cast<sStart> (s);
      if(ss && false) {
        printf("Check the start state\n");
        for(int i=0; i<ss->distr->ds->qty; i++)
          if(ss->distr->val[i]) printf("%d: %lf\n", i, ss->distr->val[i]);
        }
      }    


  for(state s: allstates) s->optimize();
  
  auto st = allstates.begin();
  for(state s: allstates) if(!s->getskip()) *(st++) = s;
  while(allstates.end() != st) allstates.pop_back();

  printf("Activating states...\n");

  for(state s: allstates) s->activate();

  printf("states in the automaton = %d\n", (int) allstates.size());
  for(state s: allstates) {
    if(&*s) std::cout << s << ": " << *s;
    if(s->ds) std::cout << ", [" << s->ds->name << "]";
    std::cout << "\n";
    }
  std::cout << "start state = " << p.start << "\n";
  
  if(1) {
    int precint = 4;
    double prec = pow(.1, precint);
  
    int N = allstates.size();
    int ms[N];
    for(int i=0; i<N; i++) ms[i] = 0;

    for(state s: allstates) {
      auto ss = std::dynamic_pointer_cast<sStart> (s);
      if(ss) {
        printf("Doing the start state\n");
        for(int i=0; i<ss->startstate->ds->qty; i++)
          queues::add(ss->startstate, i, ss->distr->val[i]);
        }
      }    

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
      
    for(state s: allstates) {
      typedef std::shared_ptr<sAccept> acceptstate;
      acceptstate sa = std::dynamic_pointer_cast<sAccept> (s);
      if(sa) {
        if(sa->distr->ds == sPaper)
          present(sa->distr, 
            viewTable(paperTitles),
            viewTable(paperyears, "year: "),
            viewTable(papertypes, "type: "),
            viewEdge(rAuthors, "authors:", viewTable(authorNames)),
            viewEdge(rWhichjournal, "journal:", viewTable(journalNames))
            );
        else if(sa->distr->ds == sAuthor)
          present(sa->distr, 
            viewTable(authorNames),
            viewEdge(rPapers, "papers:", viewTable(paperTitles))
            );
        }
      }    
    
    }
  return 0;
  }
