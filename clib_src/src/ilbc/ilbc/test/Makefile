export CROSS?=/opt/buildroot-mipsel-iplus/bin/mipsel-linux-uclibc-
CC = $(CROSS)gcc
CFLAGS=-g -O2 -Wall -I../ -I../interface
LIBS=-L./../ -L./../../signal_processing/ -lwebrtc_ilbc -lwebrtc_spl

all:
	$(CC) iLBC_test.c -o iLBC_test $(CFLAGS) $(LIBS)
