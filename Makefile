OBJ=pbrain.o spigot.o

%.o: %.c
	$(CC) -fPIC -c -g -ansi $< -o $@

spigot.so: $(OBJ)
	$(CC) -fPIC -shared -o $@ $(OBJ) -lsporth -lsoundpipe -lm -lsndfile -ldl

clean:
	rm -rf $(OBJ) spigot.so
