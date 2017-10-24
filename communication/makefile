# makefile needed
CC := gcc -D _GNU_SOURCE
CPP := gcc -E
CFLAGS := -O2 -g -Wall -Wextra -Wno-strict-aliasing -std=c99 -Iinc -I ./inc
SOURCES := $(wildcard src/*.c src/main/*.c)
HEADERS := 
OBJS := ${SOURCES:%.c=%.o}
 
# self-defined
APP_NAME := ./bin/communication
LIBS := 
 
all: $(APP_NAME)
	@echo "|---------------------------------------------------------|"
	@echo "|               communication MAKE DONE                   |"
	@echo "|---------------------------------------------------------|"
 
$(APP_NAME): $(OBJS)
	$(CXX) -o $@ $^ $(LIBS)
	
clean:
	@echo "|---------------------------------------------------------|"
	@echo "|               communication CLEAN DONE                  |"
	@echo "|---------------------------------------------------------|"
	rm -f $(OBJS) $(APP_NAME)