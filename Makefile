all: query-results.txt

.PHONY: compile alldata run all rebuild-data query-results.txt

read-dblp: read-dblp.cpp dblp.h ext.h
	g++ -std=c++1y read-dblp.cpp -o read-dblp -O3

dblp-search: dblp-search.cpp db.h dblp.h netauto.h netregex.h netedge.h dblp-edge.h ext.h parser.h
	g++ -std=c++1y dblp-search.cpp -o dblp-search -O3

alldata: data/authors.sdb data/journals.sdb data/paper-author.edb data/paper-journal.edb data/paper-proceedings.edb data/papers.sdb data/paper-types.db data/paper-years.db data/proceedings.sdb

data/authors.sdb data/journals.sdb data/paper-author.edb data/paper-journal.edb data/paper-proceedings.edb data/papers.sdb data/paper-types.db data/paper-years.db data/proceedings.sdb: xml/dblp.xml.gz
	make rebuild-data

rebuild-data: read-dblp
	mkdir -p data
	zcat xml/dblp.xml.gz | ./read-dblp

xml/dblp.xml.gz:
	mkdir -p xml
	wget http://dblp.uni-trier.de/xml/dblp.xml.gz -O xml/dblp.xml.gz

compile: dblp-search alldata

query-results.txt: dblp-search alldata query.txt
	./dblp-search < query.txt > query-results.txt


