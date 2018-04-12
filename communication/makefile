# makefile needed
CC := gcc -D _GNU_SOURCE
CPP := gcc -E
# -Ilib -Imain -Isys ->lib、main、sys文件夹下的搜索方式  -lworld->动态库
CFLAGS := -O2 -g -Wall -Wextra -Wno-strict-aliasing -std=c99 -Ilib -Imain -Isys -lworld
SOURCES := $(wildcard lib/*.c main/*.c sys/*.c)
HEADERS := 
OBJS := ${SOURCES:%.c=%.o}
 
# 生成可执行文件名称为communication
APP_NAME := ./bin/communication
LIBS := 
 
all: $(APP_NAME)
	@echo "|---------------------------------------------------------|"
	@echo "|               communication MAKE DONE                   |"
	@echo "|---------------------------------------------------------|"
# -lpthread 创建了线程
$(APP_NAME): $(OBJS) -lpthread
	$(CXX) -o $@ $^ $(LIBS)
# 删除.o文件
	rm -f $(OBJS)
	
clean:
	@echo "|---------------------------------------------------------|"
	@echo "|               communication CLEAN DONE                  |"
	@echo "|---------------------------------------------------------|"
# 删除.o 和可执行文件
	rm -f $(OBJS) $(APP_NAME)