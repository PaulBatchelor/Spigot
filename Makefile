pbrain: pbrain.c
	$(CC) -g -ansi $< -o $@

clean:
	rm -rf pbrain
