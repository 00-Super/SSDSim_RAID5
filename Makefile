.PHONY : all clean rebuild  

all: ssd
	
clean:
	rm -f ssd *.o *~

ssd: ssd.o flash.o  pagemap.o  hash.o   initialize.o
	gcc  -g -o ssd $^ -lm
%.o: %.c
	gcc -c  -g  $^ -o $@

rebuild : clean all
