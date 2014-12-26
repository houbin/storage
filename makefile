######################################################################
# Generic makefile
#
#
#######################################################################
# Customising
# 
# Adjust the following if necessary 
EXECUTABLE := storage
LIBDIR := ./lib
LIBS := pthread grpc cJSON rt stdc++ resolv m
INCLUDES := ./util ./include
UTIL_DIR := ./util
SRC_DIR := ./src

# Flags
CC := g++
CFLAGS := -g -Wall
CPPFLAGS := $(CFLAGS)
CPPFLAGS += $(addprefix -I, $(INCLUDES))
CPPFLAGS += -MMD

RM-F := rm -f

# # You should't need to change anything below this points
C_SRCS := $(wildcard grpc/*.c)
C_OBJS := $(patsubst %.c, %.o, %(C_SRCS))

SRCS := $(wildcard *.cc) $(wildcard $(addsuffix /*.cc, $(SRC_DIR))) $(wildcard $(addsuffix /*.cc, $(UTIL_DIR)))
OBJS := $(patsubst %.cc, %.o, $(SRCS))
DEPS := $(patsubst %.o, %.d, $(OBJS))
MISSING_DEPS := $(filter-out $(wildcard $(DEPS)), $(DEPS))
MISSING_DEPS_SOURCES := $(wildcard $(patsubst %d, %cc, $(MISSING_DEPS)))

.PHONY : all deps objs clean veryclean rebuild info
all : $(EXECUTABLE)

deps : $(DEPS)

c_objs : $(C_OBJS)
objs : $(OBJS)

clean :
	@$(RM-F) $(OBJS)
	@$(RM-F) $(C_OBJS)
	@$(RM-F) $(DEPS)

veryclean: clean
	@$(RM-F) $(EXECUTABLE)

rebuild: veryclean all
ifneq ($(MISSING_DEPS),)
$(MISSING_DEPS) :
	@$(RM-F) $(patsubst %d, %o, $@)
endif

-include $(DEPS)

$(EXECUTABLE) : $(OBJS) $(C_OBJS)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(OBJS) $(C_OBJS) $(addprefix -L, $(LIBDIR)) $(addprefix -l, $(LIBS))

info:
	@echo $(SRCS)
	@echo $(OBJS)
	@echo $(DEPS)
	@echo $(MISSING_DEPS)
	@echo $(MISSING_DEPS_SOURCES)
