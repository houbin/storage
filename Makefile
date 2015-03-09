#############################################################   
# Makefile for shared library.  
# # 编译动态链接库  
# #############################################################  
# #set your own environment option  
CC = g++
CC_FLAG = -g -fPIC
#   
#   #set your inc and lib
INC = -I./include -I./src/ -I./util
LIB = -lpthread -lrt -lstdc++ -lm
  
#make target lib and relevant obj
PRG = libstorage.so
STATIC_PRG = libstorage.a
OBJ = src/free_file_table.o src/id_center.o src/index_file.o src/record_file.o src/storage_api.o \
src/store_client_center.o util/clock.o util/coding.o util/cond.o util/crc32c.o util/logger.o \
util/mutex.o util/thread.o util/timer.o

DEPS = src/free_file_table.d src/id_center.d src/index_file.d src/record_file.d src/storage_api.d \
src/store_client_center.d util/clock.d util/coding.d util/cond.d util/crc32c.d util/logger.d \
util/mutex.d util/thread.d util/timer.d

#all target
all:$(PRG) $(STATIC_PRG)

$(PRG):$(OBJ)
	$(CC) -fPIC -shared -DDEBUG -o $@ $(OBJ) $(LIB)

$(STATIC_PRG):$(OBJ)
	ar -crv $@ $(OBJ)

.SUFFIXES: .c .o .cc
%.o : %.cc
	$(CC) $(CC_FLAG) $(INC) -c $*.cc -o $*.o

.PRONY:clean
clean:
	@echo "Removing linked and compiled files......;"
	-rm -f $(OBJ) $(PRG) $(DEPS)
