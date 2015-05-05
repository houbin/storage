CC = g++
CC_FLAG = -g -rdynamic -m32

INC =
LIB = -L../install -lstorage -lpthread

PRG = storage_test

OBJS = ./test_writer.o ./test_reader.o ../util/config.o  ./storage_main.o

all:$(PRG)

$(PRG):$(OBJS)
	$(CC) -m32 -o $@ $(OBJS) $(LIB)

.SUFFIXES: .cc
%.o : %.cc
	$(CC) $(CC_FLAG) $(INC) -c $*.cc -o $*.o

.PRONY:clean
clean:
	-rm -f $(PRG) $(OBJS)
