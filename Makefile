PRJ=$(shell basename $(CURDIR))

all: $(PRJ)

$(PRJ): *.c *.h
	gcc -std=c11 -I . -o $(PRJ) *.c -lm

.PHONY : clean

clean:
	-@rm -f $(PRJ) *.o *~