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

value 0 is interpreted as 'false', 1 is interpreted as 'true'
in-between values are interpreted as probabilities

x&y is equivalent to x*y
x|y is equivalent to x+y-x*y
x^y is min(x,y)

<real constant>
<condition1> (+|-|*|/|&|||^) <condition2>
<condition1> (>|<|=|!=|>=|<=) <condition2> (returns a 0/1 condition)

search <nameset> "string"
regexsearch <nameset> "extended POSIX regex"

(exists|forall|sum|prod|min|max) <edge> <condition> [not yet implemented]

## Expressions
(probably more accurately called instructions)

<expression>; <expression>
if <condition> then <expression> else <expression>
while <condition> do <expression>
<edge>
skip
accept
reject
start <condition>

## Edges

authors, papers
whichjournal, jpapers
whichproc, ppapers

## Name sets

authorName, journalName, paperTitle, proceedingsCode

## Example

start search(authorName, "Szymon Torunczyk");
papers;
while 0.9 do
  if search(paperTitle, "hyperbolic") then accept
  else (authors; papers);
reject
