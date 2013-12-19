#
#	Makefile for Michael Foley's jugglefest puzzle solution 
#	finish this comment before submitting
#
default: jugglefest 

all: lil_jugglefest _lil_jugglefest jugglefest _jugglefest audit_jugglefest

clean:
	rm -f jugglefest.html circuits.txt jugglers.txt \
	lil_jugglefest _lil_jugglefest jugglefest _jugglefest \
	$(LIL_FILES) $(BIG_FILES) \
	_lil_jugglefest* _jugglefest* \
	lil_circuits.txt lil_jugglers.txt big_circuits.txt big_jugglers.txt

jugglefest.html: 
	wget -q http://www.yodlecareers.com/puzzles/jugglefest.html -O $@

lil_circuits.txt: jugglefest.html
	html2text jugglefest.html | grep "^C C" > lil_circuits.txt

lil_jugglers.txt: jugglefest.html
	html2text jugglefest.html | grep "^J J" > lil_jugglers.txt

lil_circuits.c: lil_circuits.txt
	head -1 $? 
#	head -1 $? | od -ta
	sed -f circuits.sed lil_circuits.txt > $@
	head -1 $@
#	head -1 $@| od -ta
	
lil_jugglers.c: lil_jugglers.txt
	head -1 $? 
#	head -1 $? | od -ta
	sed -f jugglers.sed lil_jugglers.txt > $@
	head -1 $@
#	head -1 $@| od -ta
	

jugglefest.txt: 
	wget -q http://www.yodelcareers.com/puzzles/jugglefest.txt > $@

# LIL version with samples from html page describing the problem

LIL_FILES=lil_circuits.c lil_jugglers.c

_lil_jugglefest.i: jugglefest.c $(LIL_FILES)
	gcc -g -P -E -o $@ jugglefest.c -DLIL

_lil_jugglefest.c: _lil_jugglefest.i
	astyle < $? > $@

_lil_jugglefest.o: _lil_jugglefest.c
	gcc -g -c $?

_lil_jugglefest: _lil_jugglefest.o
	gcc -g -o $@ $?

lil_jugglefest: jugglefest.c $(LIL_FILES)
	gcc -g -o $@ jugglefest.c -DLIL

# BIG version with test files from page

big_circuits.c: big_circuits.txt
	head -1 $? 
#	head -1 $? | od -ta
	sed -f circuits.sed big_circuits.txt > $@
	head -1 $@
#	head -1 $@| od -ta
	
big_jugglers.c: big_jugglers.txt
	head -1 $? 
#	head -1 $? | od -ta
	sed -f jugglers.sed big_jugglers.txt > $@
	head -1 $@
#	head -1 $@| od -ta
	
big_circuits.txt: jugglefest.txt
	grep "^C C" jugglefest.txt > big_circuits.txt

big_jugglers.txt: jugglefest.txt
	grep "^J J" jugglefest.txt > big_jugglers.txt

BIG_FILES=big_circuits.c big_jugglers.c

_jugglefest.i: jugglefest.c $(BIG_FILES)
	gcc -g -P -E -o $@ jugglefest.c -DBIG

_jugglefest.c: _jugglefest.i
	astyle < $? > $@

_jugglefest.o: _jugglefest.c
	gcc -g -c $?

_jugglefest: _jugglefest.o
	gcc -g -o $@ $?

jugglefest: jugglefest.c $(BIG_FILES)
	gcc -g -time -o $@ jugglefest.c -DBIG 

audit_jugglefest: jugglefest.c $(BIG_FILES)
	gcc -g -o $@ jugglefest.c -DBIG -DPREF_AUDIT

jugglefest_output.txt: jugglefest
	./jugglefest  -o -v2 | head -20000000 | tail -100000 > $@

michaelfoley.taz: Makefile jugglefest.c circuits.sed jugglers.sed 
	tar -czf michaelfoley.taz Makefile jugglefest.c circuits.sed jugglers.sed 
