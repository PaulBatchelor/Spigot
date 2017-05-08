OBJ=pbrain.o spigot.o graphics.o runt.o
CFLAGS += -fPIC -g -ansi -Wall
LIBS += -lsporth -lsoundpipe -lm -lsndfile -ldl -lglfw -lGL -lrunt

CONFIG ?=

include $(CONFIG)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

spigot.so: $(OBJ)
	$(CC) $(CFLAGS) -shared -o $@ $(OBJ) $(LIBS)
clean:
	rm -rf $(OBJ) spigot.so
