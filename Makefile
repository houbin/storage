#############################################################   
# Makefile for shared library.  
# # 编译动态链接库  
# #############################################################  
# #set your own environment option  
CC = g++
CC_FLAG = -g -Wall -Wno-unused-parameter -Werror -fPIC -m32 
#CC_FLAG = -Wall -Wno-unused-parameter -Werror -fPIC -m32 
#-march=i686
#   
#   #set your inc and lib
INC = -I./include -I./src/ -I./util
LIB = -lpthread -lrt -lstdc++ -lm
  
#make target lib and relevant obj
PRG = libstorage.so
FORMAT_DISK=format_one_disk
STATIC_PRG = libstorage.a
OBJ = src/free_file_table.o src/id_center.o src/index_file.o src/record_file.o src/storage_api.o \
src/store_client_center.o util/clock.o util/coding.o util/cond.o util/crc32c.o util/logger.o \
util/mutex.o util/thread.o util/timer.o src/record_writer.o src/record_reader.o src/store_client.o \
util/config.o

DEPS = src/free_file_table.d src/id_center.d src/index_file.d src/record_file.d src/storage_api.d \
src/store_client_center.d util/clock.d util/coding.d util/cond.d util/crc32c.d util/logger.d \
util/mutex.d util/thread.d util/timer.d src/record_writer.d src/record_reader.d \
src/store_client.d util/config.d

all:$(PRG) $(FORMAT_DISK)

$(PRG):$(OBJ)
	#$(CC) -m32 -fPIC -shared -DDEBUG -o install/libstorage.so $(OBJ) $(LIB) -L./lib/ -ltcmalloc
	$(CC) -m32 -fPIC -shared -o install/libstorage.so $(OBJ) $(LIB)

$(FORMAT_DISK):
	$(CC) -m32 -o install/shell/format_one_disk src/format_one_disk.cc

$(STATIC_PRG):$(OBJ)
	ar -crv $@ $(OBJ)

.SUFFIXES: .c .o .cc
%.o : %.cc
	$(CC) $(CC_FLAG) $(INC) -c $*.cc -o $*.o

.PRONY:clean
clean:
	@echo "Removing linked and compiled files......;"
	-rm -f $(OBJ) $(PRG) $(DEPS) $(STATIC_PRG) install/shell/${FORMAT_DISK}
