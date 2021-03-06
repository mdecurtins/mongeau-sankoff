CPPFLAGS = -O2 -ggdb -Wall -pedantic
CFLAGS = ${CPPFLAGS}
LIBS = -lstdc++ -lm -lpq -L/usr/lib
INCDIRS = -I/usr/include/postgresql/libpq-4

all: secuencia-recursive miditotxt-recursive query notetxt.class

secuencia-recursive: 
	$(MAKE) -C secuencia

miditotxt-recursive: 
	$(MAKE) -C miditotxt

query: query.o MS.o JCS.o Sequence.o note.o Matrix.o ProbVector.o State.o gauss.o secuencia/parsemusic.o
	gcc ${CPPFLAGS} ${LIBS} query.o MS.o JCS.o Sequence.o note.o Matrix.o ProbVector.o State.o secuencia/parsemusic.o gauss.o -o query

test: test.o MS.o JCS.o Sequence.o note.o Matrix.o ProbVector.o State.o gauss.o
	gcc ${CPPFLAGS} ${LIBS} test.o MS.o JCS.o Sequence.o note.o Matrix.o ProbVector.o State.o gauss.o secuencia/parsemusic.o -o test

secuencia/parsemusic.o: ;

query.o: query.cpp MS.h Matrix.h global.h Sequence.h note.h Result.h \
  Algorithm.h JCS.h State.h parsemusic.h
	gcc ${CPPFLAGS} ${INCDIRS} -c query.cpp

test.o: test.cpp JCS.h Sequence.h note.h global.h Matrix.h Algorithm.h \
  Result.h State.h
	gcc ${CPPFLAGS} ${INCDIRS} -c test.cpp

MS.o: MS.cpp MS.h Matrix.h global.h Sequence.h note.h Result.h \
  Algorithm.h
	gcc ${CPPFLAGS} ${INCDIRS} -c MS.cpp

JCS.o: JCS.cpp JCS.h Sequence.h note.h global.h Matrix.h Algorithm.h \
  Result.h State.h gauss.h
	gcc ${CPPFLAGS} ${INCDIRS} -c JCS.cpp

Sequence.o: Sequence.cpp Sequence.h note.h global.h
	gcc ${CPPFLAGS} ${INCDIRS} -c Sequence.cpp

note.o: note.cpp note.h global.h
	gcc ${CPPFLAGS} ${INCDIRS} -c note.cpp

Matrix.o: Matrix.cpp Matrix.h global.h
	gcc ${CPPFLAGS} ${INCDIRS} -c Matrix.cpp

ProbVector.o: ProbVector.cpp ProbVector.h global.h
	gcc ${CPPFLAGS} ${INCDIRS} -c ProbVector.cpp

State.o: State.cpp State.h global.h
	gcc ${CPPFLAGS} ${INCDIRS} -c State.cpp

gauss.o: gauss.c gauss.h
	gcc ${CFLAGS} ${INCDIRS} -c gauss.c

notetxt.class: notetxt.java
	javac notetxt.java

clean:
	$(MAKE) clean -C secuencia; $(MAKE) clean -C miditotxt; rm *.o query test notetxt.class Note.class
