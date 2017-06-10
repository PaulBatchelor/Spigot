OBJ =pbrain.o spigot.o graphics.o runt.o tracker.o

CFLAGS += -fPIC -g -ansi -Wall 
CFLAGS += -I$(HOME)/.runt/include
LIBS += -lsporth -lsoundpipe -lm -lsndfile -ldl -lglfw -lGL -lrunt -ljack
LIBS += -L$(HOME)/.runt/lib

CXXFLAGS += -D__UNIX_JACK__ -fPIC -Irtaudio $(CFLAGS)

CONFIG ?=

include $(CONFIG)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

spigot.so: $(OBJ)
	$(CC) $(CFLAGS) -shared -o $@ $(OBJ) $(LIBS)

spigot: $(OBJ) main.o rtaudio/RtAudio.o
	$(CXX) $(CXXFLAGS) $(OBJ) main.o rtaudio/RtAudio.o -o $@ $(LIBS)

clean:
	rm -rf $(OBJ) spigot.so
	rm -rf main.o rtaudio/RtAudio.o
