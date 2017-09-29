#include <cstdio>
#include <algorithm>
#include <memory>
#include <iostream>
#include <deque>
#include <cmath>
#include <queue>

#define QLEVELS 16 // more appropriate for concurrency

#include "sscholar.h"
#include "netedge.h"
#include "parser.h"

using namespace sscholar;

static relation rAuthors, rPapers, rWhichvenue, rVPapers, rCites, rCitedBy, rKeyphrases, rKPapers;


void buildRelations() {
  readEdges(sPaper, sAuthor, rAuthors, "authors", rPapers, "papers", paperAuthor);
  readEdges(sPaper, sVenue, rWhichvenue, "whichjournal", rVPapers, "jpapers", paperVenue);
  readEdges(sPaper, sPaper, rCites, "cites", rCitedBy, "citedby", citations);
  readEdges(sPaper, sKey, rKeyphrases, "keyphrases", rKPapers, "kpapers", paperKeyphrase);
  }

struct sscholarparser: parser {

  sscholarparser(std::string _s): parser(_s) {}
  
  relation getrelation(std::string s) {
    static auto allRelations = { 
      rAuthors, rPapers, rWhichvenue, rVPapers, rCites, rCitedBy, rKeyphrases, rKPapers
      };
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
    return false;
    }
  
  stringtable getstringtable(std::string s) {
    auto namedTables = {
      make_pair("authorName", authorNames),
      make_pair("venue", venues),
      make_pair("paperTitle", paperTitles),
      make_pair("paperId", paperIds),
      make_pair("keyphrase", keyphrases)
      };
    for(auto x: namedTables) if(x.first == s) return x.second;
    return nullptr;
    }
  };

int main() {

  initSorts();
  printf("Reading the database...\n");
  fflush(stdout);
  readDatabases();
  printf("Building the relations...\n");
  fflush(stdout);
  buildRelations();
  
  std::string s;
  
  printf("Parsing the expression...\n");
  fflush(stdout);
  int ch;
  while((ch = getchar()) != -1)
    s += ch;
    
  sscholarparser parser(s);
  
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
  fflush(stdout);

  allstates.clear();
  auto p = x->compile(nosort);
  if(p.finish != noleave) {
    printf("Error: leaves\n");
    exit(1);
    }
  
  printf("Optimizing the automaton...\n");
  fflush(stdout);

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
  fflush(stdout);

  for(state s: allstates) s->activate();

  printf("states in the automaton = %d\n", (int) allstates.size());
  for(state s: allstates) {
    if(&*s) std::cout << s << ": " << *s;
    if(s->ds) std::cout << ", [" << s->ds->name << "]";
    std::cout << "\n";
    }
  std::cout << "start state = " << p.start << "\n";
  fflush(stdout);
  
  if(1) {
    int precint = 4;
    double prec = pow(.1, precint);
  
    int N = allstates.size();
    int ms[N];
    for(int i=0; i<N; i++) ms[i] = 0;

    if(1) {
      printf("Start optimization loading...\n");
      for(state s: allstates) {
        auto ss = std::dynamic_pointer_cast<sStart> (s);
        if(ss) {
          printf("Doing the start state\n");
          for(int i=0; i<ss->startstate->ds->qty; i++)
            queues::add(ss->startstate, i, ss->distr->val[i]);
          }
        }    
  
      printf("Start optimization running...\n");
      queues::run();
      printf("Start optimization complete\n");
      }
    else {
      printf("Loading the start states\n");
      for(state s: allstates) {
        auto ss = std::dynamic_pointer_cast<sStart> (s);
        if(ss) {
          ext::parallelize(ss->startstate->ds->qty, [&] (int a, int b) {
            for(int i=a; i<b; i++) ss->startstate->distr->val[i] += ss->distr->val[i];
            return 0;
            });
          }
        }    
      }
  
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
      for(state& s: allstates) if(s->filled) s->energy = 0;
      for(state& s: allstates) if(s->filled) {
        auto t1 = getVa();
        s->run();
        energy += s->energy;
        auto t2 = getVa();
        ms[i++] += t2-t1;
        }
      std::cout << "Iteration #" << it << ": " << energy << " | " << getVa()-zero << "\n";
      if(energy < prec) break;
      fflush(stdout);
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
            viewTable(paperIds, "Semantic Scholar link: https://www.semanticscholar.org/paper/x/"),
            viewEdge(rAuthors, "authors:", viewTable(authorNames)),
            viewEdge(rWhichvenue, "venue:", viewTable(venues)),
            viewEdge(rKeyphrases, "keyphrases:", viewTable(keyphrases))
            );
        else if(sa->distr->ds == sAuthor)
          present(sa->distr, 
            viewTable(authorNames),
            viewEdge(rPapers, "papers:", viewTable(paperTitles))
            );
        else if(sa->distr->ds == sKey)
          present(sa->distr, 
            viewTable(keyphrases)
            );
        else if(sa->distr->ds == sVenue)
          present(sa->distr, 
            viewTable(venues)
            );
        }
      }
    
    }
  return 0;
  }
