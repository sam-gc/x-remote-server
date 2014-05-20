server: server.c
	gcc -o server server.c -lX11 -lXtst -lm 

clean:
	rm server

deploy:
	cp server ~/bin/xserv 
