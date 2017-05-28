OBJ=pbrain.o spigot.o graphics.o runt.o tracker.o
CFLAGS += -fPIC -g -ansi -Wall 
CFLAGS += -I$(HOME)/.runt/include
LIBS += -lsporth -lsoundpipe -lm -lsndfile -ldl -lglfw -lGL -lrunt
LIBS += -L$(HOME)/.runt/lib

CONFIG ?=

include $(CONFIG)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

spigot.so: $(OBJ)
	$(CC) $(CFLAGS) -shared -o $@ $(OBJ) $(LIBS)
clean:
	rm -rf $(OBJ) spigot.so
