OBJ =pbrain.o runt.o tracker.o

CFLAGS += -fPIC -g -ansi -Wall 
CFLAGS += -I$(HOME)/.runt/include
CFLAGS += -DLIVE_CODING
LIBS += -lsporth -lsoundpipe -lm -lsndfile -ldl -lglfw -lGL -lrunt -ljack
LIBS += -L$(HOME)/.runt/lib

CXXFLAGS += -D__UNIX_JACK__ -fPIC -Irtaudio $(CFLAGS)

CONFIG ?=

include $(CONFIG)

default: spigot

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

spigot.so: spigot.c graphics.c $(OBJ) 
	$(CC) $(CFLAGS) -DBUILD_SPORTH_PLUGIN spigot.c graphics.c -shared -o $@ $(OBJ) $(LIBS)

spigot: $(OBJ) main.o rtaudio/RtAudio.o spigot.o graphics.o
	$(CXX) $(CXXFLAGS) $(OBJ) spigot.o graphics.o main.o rtaudio/RtAudio.o -o $@ $(LIBS)

install: spigot
	install spigot /usr/local/bin

clean:
	rm -rf $(OBJ) spigot.so
	rm -rf main.o rtaudio/RtAudio.o spigot.o graphics.o
	rm -rf spigot
