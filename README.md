# regnetsearch
Search a relational database using a regular expression on paths from the given vertex. Based on an idea by Diego Figueira and Szymon Toruńczyk.

Parameters:
- a database describing a graph
- a vertex where we start our search
- a condition: what kind of vertices are we interested at?
- what paths from the start vertex to an accepting vertex are allowed, given by a sort-of regular expression

The current version searches DBLP -- for the given author and topic (i.e., a substring of the paper's title) you can find the papers about the topic which are the closest to the author.

Best run with 'make run' -- this will download DBLP using wget, run the parser (read-dblp), and then run the program (dblp-search).
