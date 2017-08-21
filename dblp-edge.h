#ifndef _DBLP_EDGE_H_
#define _DBLP_EDGE_H_

#include "dblp.h"
#include "netedge.h"

namespace dblp {

  static relation rAuthors, rPapers, rWhichjournal, rJPapers, rWhichproc, rPPapers;
  
  void buildRelations() {
    readEdges(sPaper, sAuthor, rAuthors, "authors", rPapers, "papers", paperAuthor);
    readEdges(sPaper, sJournal, rWhichjournal, "whichjournal", rJPapers, "jpapers", paperJournal);
    readEdges(sPaper, sProcs, rWhichproc, "whichproc", rPPapers, "ppapers", paperProceedings);
    }
  }

#endif