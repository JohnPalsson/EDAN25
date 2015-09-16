NSYM	= 1000
NVERTEX	= 100
MAXSUCC	= 4
NACTIVE	= 100
PRINT   = 1

all: compile run

compile:
	@mkdir -p classes
#	scalac -g:vars -d classes dataflow.scala
#	scalac -deprecation -d classes dataflow.scala
	scalac -deprecation -optimise -d classes dataflow.scala

run:
	scala -classpath classes Driver $(NSYM) $(NVERTEX) $(MAXSUCC) $(NACTIVE) $(PRINT)

test:
	scala -classpath classes Driver $(NSYM) $(NVERTEX) $(MAXSUCC) $(NACTIVE) $(PRINT) > test.log
	diff -u ../lab1/correct test.log
