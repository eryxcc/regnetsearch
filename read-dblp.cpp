#include <cstdio>
#include <cstdlib>

#include "dblp.h"

using namespace std;
using namespace dblp;

shared_ptr<dictionary> dAuthors, dJournals, dPapers, dProceedings;

string readTag() {
  string tag;
  bool more = true;
  while(true) {
    char ch = getchar();
    if(ch == '>') return tag;
    else if(ch == ' ') more = false;
    else if(more) tag += ch;
    }
  }

map<string, int> counts;

void readAny(string id) {
  counts[id]++;
  while(true) {
    char ch = getchar();
    if(ch == '<') {
      string s = readTag();
      if(s[0] == '/') return;
      else readAny(id+"/"+s);
      }
    }
  }

string readIn() {
  string out = "";
  while(true) {
    char ch = getchar();
    if(ch == '<') {
      string s = readTag();
      if(s[0] == '/') return out;
      else readAny("IN/"+s);
      }
    else out += ch;
    }
  }

int findoradd(stringtable st, const char* s) {
  return dynamic_pointer_cast<dictionary> (st)->findoradd(s);
  }

int justadd(stringtable st, const char* s) {
  return dynamic_pointer_cast<dictionary> (st)->add(s);
  }

void readpaper(char type, const string& s0) {
  int art = -1, journal = -1, year = 0, crossref = -1;
  vector<int> authorlist;
  while(true) {
    char ch = getchar();
    if(ch == '<') {
      string s = readTag();
      if(s == "title" && art == -1) 
        art = justadd(paperTitles, readIn().c_str());
      else if(s == "author") 
        authorlist.push_back(findoradd(authorNames, readIn().c_str()));
      else if(s == "year") 
        year = atoi(readIn().c_str());
      else if(s == "journal" && journal == -1) 
        journal = findoradd(journalNames, readIn().c_str());
      else if(s == "crossref" && crossref == -1) 
        crossref = findoradd(proceedingsCodes, readIn().c_str());
      else if(s[0] == '/') {
        if(art == -1) { printf("title missing\n"); return; }
        (*papertypes)[art] = type;
        (*paperyears)[art] = year;
        if(journal != -1) paperJournal.add(art, journal);
        if(crossref != -1) paperProceedings.add(art, crossref);
        for(int aut: authorlist) paperAuthor.add(art, aut);
        return;
        }
      else readAny(s0+s);
      }
    }
  }  

size_t last;

void readDBLP() {
  while(true) {
    char ch = getchar();
    if(ch == '<') {
      string s = readTag();
      if(s == "article") readpaper('a', "dblp/article/");
      else if(s == "inproceedings") readpaper('c', "dblp/inproceedings/");
      else if(s[0] == '/') return;
      else readAny("dblp/"+s);
      }
//    size_t s = ftell(stdin);
//    if(s > last + 20000000) { printf("%d\n", (int) s); last = s; }
//    if(s > 40000000) return;
    }
  }

int main() {

  dblp::initSorts();

  authorNames = make_shared<dictionary> (sAuthor);
  journalNames = make_shared<dictionary> (sJournal);
  paperTitles = make_shared<dictionary> (sPaper);
  proceedingsCodes = make_shared<dictionary> (sProcs);
  
  papertypes = make<podtable<char>> (sPaper);
  paperyears = make<podtable<int>> (sPaper);
  
  char header[80];
  for(int i=0; i<2; i++) fgets(header, 80, stdin);
  char ch = getchar();
  if(ch != '<') { printf("bad format\n"); exit(1); }
  string s = readTag();
  if(s != "dblp") { printf("bad main tag\n"); exit(1); }
  else readDBLP();
  for(auto p: counts)
    printf("%s: %d\n", p.first.c_str(), p.second);

  dblp::writeDatabases();
  return 0;
  }
