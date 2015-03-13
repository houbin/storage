CC = g++
CC_FLAG = -g -rdynamic

INC =
LIB = -L../ -lstorage -lpthread

PRG = storage_test

OBJS = ./test_writer.o ./test_reader.o ../util/config.o  ./storage_main.o

all:$(PRG)

$(PRG):$(OBJS)
	$(CC) -o $@ $(OBJS) $(LIB)

.SUFFIXES: .cc
%.o : %.cc
	$(CC) $(CC_FLAG) $(INC) -c $*.cc -o $*.o

.PRONY:clean
clean:
	-rm -f $(PRG) $(OBJS)
