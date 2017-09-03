OBJ =pbrain.o runt.o tracker.o 

CFLAGS += -fPIC -g -ansi -Wall 
CFLAGS += -I$(HOME)/.runt/include
CFLAGS += -DLIVE_CODING
LIBS += -lsporth -lsoundpipe -lm -lsndfile -ldl -lglfw -lGL -lrunt -ljack
LIBS += -L$(HOME)/.runt/lib
LIBS += -lrunt_plumber

CXXFLAGS += -D__UNIX_JACK__ -fPIC -Irtaudio $(CFLAGS)

CONFIG ?=

include $(CONFIG)

default: spigot libspigot.a

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

spigot.so: spigot.c graphics.c plugin.c $(OBJ) 
	$(CC) $(CFLAGS) -DBUILD_SPORTH_PLUGIN spigot.c graphics.c plugin.c -shared -o $@ $(OBJ) $(LIBS)

spigot: $(OBJ) main.o rtaudio/RtAudio.o spigot.o graphics.o plugin.o
	$(CXX) $(CXXFLAGS) $(OBJ) spigot.o graphics.o main.o rtaudio/RtAudio.o plugin.o -o $@ $(LIBS)

libspigot.a: $(OBJ) spigot.o graphics.o
	$(AR) rcs $@ $(OBJ) spigot.o graphics.o

install: spigot libspigot.a 
	install spigot /usr/local/bin
	install libspigot.a /usr/local/lib
	install spigot.h /usr/local/include

clean:
	rm -rf $(OBJ) spigot.so
	rm -rf main.o rtaudio/RtAudio.o spigot.o graphics.o plugin.o
	rm -rf spigot
