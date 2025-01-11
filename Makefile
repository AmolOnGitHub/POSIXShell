run: *.c shell_methods/*.c shell_methods/methods/*.c
	gcc -o shell *.c shell_methods/*.c shell_methods/methods/*.c 
	./shell

clean:
	rm -f shell