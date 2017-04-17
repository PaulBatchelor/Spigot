OBJ=pbrain.o spigot.o
CFLAGS += -fPIC -g -ansi
LIBS += -lsporth -lsoundpipe -lm -lsndfile -ldl


%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

spigot.so: $(OBJ)
	$(CC) $(CFLAGS) -shared -o $@ $(OBJ) $(LIBS)
clean:
	rm -rf $(OBJ) spigot.so
