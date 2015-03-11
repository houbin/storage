CC = g++
CC_FLAG = -g -rdynamic

INC =
LIB = -L../ -lstorage -lpthread

PRG = write_test

OBJS = ./storage_test.o ./storage_main.o ../util/config.o

all:$(PRG)

$(PRG):$(OBJS)
	$(CC) -o $@ $(OBJS) $(LIB)

.SUFFIXES: .cc
%.o : %.cc
	$(CC) $(CC_FLAG) $(INC) -c $*.cc -o $*.o

.PRONY:clean
clean:
	-rm -f $(PRG) $(OBJS)
