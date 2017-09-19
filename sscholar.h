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
    binread("data-sscholar/id-paper.sdb", paperIds, sPaper);
    binread("data-sscholar/paper-citations.edb", citations);

    binread("data-sscholar/papers.sdb", paperTitles, sPaper);

    binread("data-sscholar/authors.sdb", authorNames, sAuthor);    
    binread("data-sscholar/paper-author.edb", paperAuthor);
    
    binread("data-sscholar/paper-years.db", paperyears, sPaper);

    binread("data-sscholar/venues.sdb", venues, sVenue);    
    binread("data-sscholar/paper-venue.edb", paperVenue);

    binread("data-sscholar/keyphrases.sdb", keyphrases, sKey);    
    binread("data-sscholar/paper-keyphrases.edb", paperKeyphrase);
    }
  
  }

#endif
