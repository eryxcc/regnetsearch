#include <iostream>
#include "json.hpp"

#include "db.h"
#include "sscholar.h"

int whichphases = 0;

bool PH(int i) { return ((whichphases >> i)&1); }

int main(int argc, char **argv) {

  if(argc == 1) whichphases = -1;
  
  for(int i=1; i<argc; i++)
    whichphases |= 1<<atoi(argv[i]);

  using namespace sscholar;
  using namespace std;
  using json = nlohmann::json;

  initSorts();
  
  auto paperIds = make_shared<dictionary> (sPaper);
  auto authorNames = make_shared<dictionary> (sAuthor);
  auto venues = make_shared<dictionary> (sVenue);
  auto keyphrases = make_shared<dictionary> (sKey);
  auto paperTitles = make_shared<stringstable> (sPaper);
  auto paperAbstracts = make_shared<stringstable> (sPaper);
  paperyears = make<podtable<int>> (sPaper);

  while(true) {
    json j;
    std::string s;
    getline(std::cin, s);
    if(s == "") break;
    j = json::parse(s);
    
    int id = paperIds->findoradd(j["id"]);

    if(PH(1)) for(string k: j["outCitations"]) {
      int id2 = paperIds->findoradd(k);
      citations.add(id, id2);
      }
  
    if(PH(2)) paperTitles->set(id, j["title"].get<string>());
    
    if(PH(7) && j.count("paperAbstracts")) paperAbstracts->set(id, j["paperAbstract"].get<string>());
    
    if(PH(3)) for(auto a: j["authors"]) {
      int aid = authorNames->findoradd(a["name"].get<string>());
      paperAuthor.add(id, aid);
      printf("  author id = %d\n", aid);
      }
    
    if(PH(4) && j.count("year"))
      (*paperyears)[id] = j["year"].get<int>();
    
    if(PH(5)) {
      string vstr = j["venue"];    
      if(vstr != "") {
        int venueid = venues->findoradd(vstr);
        paperVenue.add(id, venueid);
        }
      }
      
   if(PH(6)) for(string k: j["keyPhrases"]) {
      int keyid = keyphrases->findoradd(k);
      paperKeyphrase.add(id, keyid);
      }

    }
  
  if(PH(1)) { 
    binwrite("data-sscholar/id-paper.sdb", (stringtable) paperIds);    
    binwrite("data-sscholar/paper-citations.edb", citations);
    }

  if(PH(2))
    binwrite("data-sscholar/papers.sdb", (stringtable) paperTitles);

  if(PH(3)) {
    binwrite("data-sscholar/authors.sdb", (stringtable) authorNames);
    binwrite("data-sscholar/paper-author.edb", paperAuthor);
    }
       
  if(PH(4)) {
    binwrite("data-sscholar/paper-years.db", paperyears);
    }
  
  if(PH(5)) {
    binwrite("data-sscholar/venues.sdb", (stringtable) venues);
    binwrite("data-sscholar/paper-venue.edb", paperVenue);
    }
  
  if(PH(6)) {
    binwrite("data-sscholar/keyphrases.sdb", (stringtable) keyphrases);
    binwrite("data-sscholar/paper-keyphrases.edb", paperKeyphrase);
    }
  
  if(PH(7))
    binwrite("data-sscholar/paper-abstracts.sdb", (stringtable) paperAbstracts);
  
  printf("done\n");
       
  return 0;
  }
