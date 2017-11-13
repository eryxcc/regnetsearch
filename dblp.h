#ifndef _DBLP_H_
#define _DBLP_H_

#include "db.h"

namespace dblp {

  using namespace regnetsearch;

  static dbsort 
    sAuthor, sPaper, sJournal, sProcs;
  
  podtable<char> papertypes;
  podtable<int> paperyears;

  stringtable authorNames, journalNames, paperTitles, proceedingsCodes;

  edgedb paperAuthor;
  edgedb paperJournal;
  edgedb paperProceedings;

  void initSorts() {
    sPaper = make<dbsort>("paper");
    sAuthor = make<dbsort>("author");
    sJournal = make<dbsort>("journal");
    sProcs = make<dbsort>("proceedings");
    }
  
  void readDatabases() {
    lazy_read("data/papers.sdb", paperTitles, sPaper);
    lazy_read("data/authors.sdb", authorNames, sAuthor);
    lazy_read("data/journals.sdb", journalNames, sJournal);
    lazy_read("data/proceedings.sdb", proceedingsCodes, sProcs);
    
    lazy_read("data/paper-author.edb", paperAuthor);
    lazy_read("data/paper-journal.edb", paperJournal);
    lazy_read("data/paper-proceedings.edb", paperProceedings);
    
    lazy_read("data/paper-types.db", papertypes, sPaper);
    lazy_read("data/paper-years.db", paperyears, sPaper);
    }
  
  void writeDatabases() {
    binwrite("data/papers.sdb", paperTitles);
    binwrite("data/authors.sdb", authorNames);
    binwrite("data/journals.sdb", journalNames);
    binwrite("data/proceedings.sdb", proceedingsCodes);
    
    binwrite("data/paper-author.edb", paperAuthor);
    binwrite("data/paper-journal.edb", paperJournal);
    binwrite("data/paper-proceedings.edb", paperProceedings);

    binwrite("data/paper-types.db", papertypes);
    binwrite("data/paper-years.db", paperyears);
    }

  }

#endif
