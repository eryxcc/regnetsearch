#ifndef _DBLP_H_
#define _DBLP_H_

#include "db.h"

namespace sscholar {

  using namespace regnetsearch;

  static dbsort 
    sAuthor, sPaper, sKey, sVenue;
  
  podtable<int> paperyears;

  stringtable authorIds, paperIds, authorNames, paperTitles, keyphrases, venues;

  edgedb paperAuthor, paperKeyphrase, paperVenue, citations;

  void initSorts() {
    sPaper = make<dbsort>("paper");
    sAuthor = make<dbsort>("author");
    sKey = make<dbsort>("keyphrase");
    sVenue = make<dbsort>("venue");
    }
  
  void readDatabases() {
    binread("data-sscholar/papers.sdb", paperTitles, sPaper);
    binread("data-sscholar/authors.sdb", authorNames, sAuthor);
    binread("data-sscholar/id-paper.sdb", paperIds, sPaper);
    binread("data-sscholar/id-author.sdb", authorIds, sAuthor);
    
    binread("data-sscholar/paper-author.edb", paperAuthor);
    
    binread("data-sscholar/paper-years.db", paperyears, sPaper);
    }
  
  void writeDatabases() {
    binwrite("data-sscholar/papers.sdb", paperTitles);
    binwrite("data-sscholar/authors.sdb", authorNames);
    binwrite("data-sscholar/id-paper.sdb", paperIds);
    binwrite("data-sscholar/id-author.sdb", authorIds);
    binwrite("data-sscholar/keyphrases.db", keyphrases);
    binwrite("data-sscholar/venues.db", venues);

    binwrite("data-sscholar/paper-author.edb", paperAuthor);
    binwrite("data-sscholar/paper-keyphrases.edb", paperKeyphrase);
    binwrite("data-sscholar/paper-venue.edb", paperVenue);
    binwrite("data-sscholar/paper-citations.edb", citations);

    binwrite("data-sscholar/paper-years.db", paperyears);
    }

  }

#endif
