#ifndef _DBLP_H_
#define _DBLP_H_

#include "db.h"

namespace sscholar {

  using namespace regnetsearch;

  static dbsort 
    sAuthor, sPaper, sKey, sVenue;
  
  podtable<int> paperyears;

  stringtable authorIds, paperIds, authorNames, paperTitles, keyphrases, venues, paperAbstracts;

  edgedb paperAuthor, paperKeyphrase, paperVenue, citations;

  void initSorts() {
    sPaper = make<dbsort>("paper");
    sAuthor = make<dbsort>("author");
    sKey = make<dbsort>("keyphrase");
    sVenue = make<dbsort>("venue");
    }
  
  void readDatabases() {
    lazy_read("data-sscholar/id-paper.sdb", paperIds, sPaper);
    lazy_read("data-sscholar/paper-citations.edb", citations);

    lazy_read("data-sscholar/papers.sdb", paperTitles, sPaper);

    lazy_read("data-sscholar/paper-abstracts.sdb", paperAbstracts, sPaper);

    lazy_read("data-sscholar/authors.sdb", authorNames, sAuthor);    
    lazy_read("data-sscholar/paper-author.edb", paperAuthor);
    
    lazy_read("data-sscholar/paper-years.db", paperyears, sPaper);

    lazy_read("data-sscholar/venues.sdb", venues, sVenue);    
    lazy_read("data-sscholar/paper-venue.edb", paperVenue);

    lazy_read("data-sscholar/keyphrases.sdb", keyphrases, sKey);    
    lazy_read("data-sscholar/paper-keyphrases.edb", paperKeyphrase);
    }
  
  }

#endif
