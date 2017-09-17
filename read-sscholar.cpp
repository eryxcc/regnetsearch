#include <iostream>
#include "json.hpp"

#include "db.h"
#include "sscholar.h"

int main() {

  using namespace sscholar;
  using namespace std;
  using json = nlohmann::json;

  initSorts();

  auto authorNames = make_shared<dictionary> (sAuthor);
  auto venues = make_shared<dictionary> (sVenue);
  auto keyphrases = make_shared<dictionary> (sKey);
  auto paperTitles = make_shared<stringstable> (sPaper);
  auto authorIds = make_shared<dictionary> (sAuthor);
  auto paperIds = make_shared<dictionary> (sPaper);
  paperyears = make<podtable<int>> (sPaper);

  json j;
  while(true) {
    std::string s;
    getline(std::cin, s);
    if(s == "") break;
    j = json::parse(s);
    
    int id = paperIds->findoradd(j["id"].get<string>());
    paperTitles->set(id, j["title"].get<string>());
    
    for(auto a: j["authors"]) {
      int aid = authorNames->findoradd(a["name"].get<string>());
      paperAuthor.add(id, aid);
      }
    
    if(j.count("year"))
      (*paperyears)[id] = j["year"].get<int>();
    
    string vstr = j["venue"];
    
    if(vstr != "") {
      int venueid = venues->findoradd(vstr);
      paperVenue.add(id, venueid);
      }

    for(string k: j["keyPhrases"]) {
      int keyid = keyphrases->findoradd(k);
      paperKeyphrase.add(id, keyid);
      }

    for(string k: j["outCitations"]) {
      int id2 = paperIds->findoradd(k);
      citations.add(id, id2);
      }
    }
       
  sscholar::authorNames = authorNames;
  sscholar::paperTitles = paperTitles;
  sscholar::authorIds = authorIds;
  sscholar::paperIds = paperIds;
  sscholar::venues = venues;
  sscholar::keyphrases = keyphrases;
  printf("Writing the databases\n");
  writeDatabases();
  return 0;
  }
