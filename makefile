CXX			:=	-c++
CXXFLAGS	:=	-std=c++20 -Wall -Wextra -Werror -Wpedantic
LDFLAGS		:=  -lstdc++ -lstdc++fs
BUILD		:=	./build
OBJ_DIR		:=	$(BUILD)/objects
APP_DIR		:=	$(BUILD)/apps
TARGET		:=	program
INCLUDE		:=	-Iinclude -I/usr/include
SRC			:=	$(wildcard	src/*.cpp)

OBJECTS		:=	$(SRC:%.cpp=$(OBJ_DIR)/%.o)
PROGS		:=	$(patsubst	%.cpp,$(APP_DIR)/%,$(SRC))

all: build $(PROGS)

$(OBJ_DIR)/%.o:	%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -MMD -o $@

# $(APP_DIR)/$(TARGET): $(OBJECTS)
$(APP_DIR)/%: $(OBJ_DIR)/%.o
	@mkdir -p $(@D)
# 	$(CXX) $(CXXFLAGS) -o $(APP_DIR)/$(TARGET) $^ $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: all build clean debug release info

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -DDEBUG -g
debug: clean all

release: CXXFLAGS += -O2
release: clean all

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*

info:
	@echo	"[*]	App dir:		${APP_DIR}	"
	@echo	"[*]	Object dir:		${OBJ_DIR}	"
	@echo	"[*]	Sources:		${SRC}		"
	@echo	"[*]	Objects:		${OBJECTS}	"
	@echo	"[*]	Programs:		${PROGS}	"