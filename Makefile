build:
	gcc image_editor.c -o image_editor -Wall -lm

run:
	./image_editor

clean:
	rm image_editor