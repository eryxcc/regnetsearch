# regnetsearch
Search a relational database using a regular expression on paths from the given vertex. Based on an idea by Diego Figueira and Szymon Toruńczyk.

Parameters:
- a database describing a graph
- a vertex where we start our search
- a condition: what kind of vertices are we interested at?
- what paths from the start vertex to an accepting vertex are allowed, given by a sort-of regular expression

The current version searches DBLP -- for the given author and topic (i.e., a substring of the paper's title) you can find the papers about the topic which are the closest to the author.

Best run with 'make' -- this will download DBLP using wget, run the parser (read-dblp), and then run the program on the sample query (dblp-search < query.txt > query-result.txt).

# Query language

## Conditions

value 0 is interpreted as 'false', 1 is interpreted as 'true'; in-between values are interpreted as probabilities

x&y is equivalent to x\*y

x|y is equivalent to x+y-x\*y

x^y is min(x,y)

* _real constant_
* _condition1_ (+|-|\*|/|&|||^) _condition2_
* _condition1_ (>|<|=|!=|>=|<=) _condition2_ (returns a 0/1 condition)
* search _nameset_ "string"
* regexsearch _nameset_ "extended POSIX regex"
* (exists|forall|sum|prod|min|max) _edge_ _condition_ [not yet implemented]
* _table_

## Expressions
(probably more accurately called instructions)

* _expression_; _expression_
* if _condition_ then _expression_ else _expression_
* while _condition_ do _expression_
* _edge_
* skip
* accept
* reject
* start _condition_

## DBLP database

### Edges

authors, papers
whichjournal, jpapers
whichproc, ppapers

### Name sets

authorName, journalName, paperTitle, proceedingsCode

### Tables 

paperYear, paperType

## Semantic Scholar database

### Edges

authors, papers
whichvenue, vpapers
cites, citedby
keyphrases, kpapers

### Name sets

authorName, paperTitle, proceedingsCode, venue, keyphrase, paperId

### Tables 

paperYear

## Example

    start search(authorName, "Szymon Torunczyk");
    papers;
    while 0.9 do
      if search(paperTitle, "hyperbolic") then accept
      else (authors; papers);
    reject
