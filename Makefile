.PHONY: osx linux
OBJ =pbrain.o runt.o tracker.o db.o audio.o step.o

CFLAGS += -fPIC -g -Wall 
CFLAGS += -I$(HOME)/.runt/include
CFLAGS += -DLIVE_CODING
LIBS += -lsporth -lsoundpipe -lm -lsndfile -ldl -lglfw -lrunt
LIBS += -L$(HOME)/.runt/lib
LIBS += -lrunt_plumber
LIBS += -lsqlite3

CONFIG ?=

include $(CONFIG)

default: ; @echo "Usage: make [osx|linux]"

all: spigot libspigot.a

osx: ; make -f Makefile -f Makefile.osx all

linux: ; make -f Makefile -f Makefile.linux all

%.o: %.c
	$(CC) -ansi -c $(CFLAGS) $< -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

spigot: $(OBJ) main.o rtaudio/RtAudio.o spigot.o graphics.o plugin.o
	$(CXX) $(CXXFLAGS) $(OBJ) spigot.o graphics.o main.o rtaudio/RtAudio.o plugin.o -o $@ $(LIBS)

libspigot.a: $(OBJ) spigot.o graphics.o rtaudio/RtAudio.o
	$(AR) rcs $@ $(OBJ) spigot.o graphics.o rtaudio/RtAudio.o

test_db: test_db.c libspigot.a
	$(CC) $(CFLAGS) $< -o $@ libspigot.a $(LIBS)

install: spigot libspigot.a 
	install spigot /usr/local/bin
	install libspigot.a /usr/local/lib
	install spigot.h /usr/local/include

clean:
	rm -rf $(OBJ) spigot.so
	rm -rf main.o rtaudio/RtAudio.o spigot.o graphics.o plugin.o
	rm -rf spigot
	rm -rf libspigot.a
