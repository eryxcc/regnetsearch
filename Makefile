all: query-results.txt

.PHONY: compile alldata run all rebuild-data query-results.txt

read-dblp: read-dblp.cpp dblp.h ext.h db.h
	g++ -std=c++1y read-dblp.cpp -o read-dblp -O3 -lpthread

dblp-search: dblp-search.cpp db.h dblp.h netauto.h netregex.h netedge.h ext.h parser.h
	g++ -std=c++1y dblp-search.cpp -o dblp-search -O3 -lpthread

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

read-sscholar: read-sscholar.cpp db.h sscholar.h
	g++ -std=c++1y read-sscholar.cpp -o read-sscholar -O3 -lpthread

rebuild-data-sscholar: read-sscholar
	mkdir -p data-sscholar
	zcat sscholar/papers-*.json.gz | ./read-sscholar

rebuild-data-sscholar-t: read-sscholar
	mkdir -p data-sscholar
	zcat sscholar/papers-*.json.gz | /usr/bin/time ./read-sscholar

data-sscholar/papers.sdb:
	$(info You need to download the data from Semantic Scholar for sscholar-search to work.)
	$(info 1. download the corpus from http://labs.semanticscholar.org/corpus/)
	$(info 2. unzip the zip, and place the papers-*.json.gz obtained in sscholar)
	$(info 3. make rebuild-data-scholar)
	$(error no data found)

sscholar-search: sscholar-search.cpp db.h sscholar.h netauto.h netregex.h netedge.h ext.h parser.h 
	g++ -std=c++1y sscholar-search.cpp -o sscholar-search -O3 -lpthread

sscholar-query-results.txt: sscholar-search data-sscholar/papers.sdb query.txt
	./sscholar-search < query.txt > sscholar-query-results.txt
