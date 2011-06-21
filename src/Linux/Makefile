CC=gcc
PROG=su_dbg_win

PL=
LIBS=`pkg-config gtk+-2.0 --libs`
OBJS=callbacks.o interface.o support.o su_dbg_win.o
CFLAGS=-Wall -O3 `pkg-config gtk+-2.0 --cflags` -I../ -I./ -D_REENTRANT -DSU_ENABLE_ANSI_CODE -pthread

all: $(PROG)

.c.o:
	$(CC) $(CFLAGS) -c $<

su_dbg_win.o: ../Windows/su_dbg_win/su_dbg_win.c
	$(CC) $(CFLAGS) -c ../Windows/su_dbg_win/su_dbg_win.c

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(LIBS) $(OBJS) $(LIBS) $(CFLAGS)

clean:
	rm -f *.o $(PROG)

