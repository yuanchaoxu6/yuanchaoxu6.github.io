all: benchmark validate

benchmark: benchmark.c 
	$(CC) -g -O0 linkedlist.c -o link -I/usr/local/include -lmcontainer
	
validate: validate.c 
	$(CC) -g -O0 validate.c -o validate -lmcontainer
	
clean:
	rm -f benchmark validate
