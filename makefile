CXX      := -g++
CXXFLAGS := -std=c++14 -pedantic-errors -fopenmp
LDFLAGS  := -L/usr/lib64 -lstdc++
BUILD    := ./build
OBJ_DIR  := $(BUILD)/objects
APP_DIR  := $(BUILD)/apps
TARGET   := coloring
INCLUDE  := -Iinclude/
SRC      :=                      \
   $(wildcard src/*/*.c) \
   $(wildcard src/*.cpp)         \

OBJECTS  := $(SRC:%.cpp=$(OBJ_DIR)/%.o)
DEPENDENCIES := $(OBJECTS:.o=.d)

all: build $(APP_DIR)/$(TARGET)
$(OBJ_DIR)/%.o: %.c*
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -MMD -o $@

$(APP_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(APP_DIR)/$(TARGET) $^ $(LDFLAGS)

.PHONY: all build clean debug release info extract purge bone010 nlpkkt120 nlpkkt240

extract:
	tar -xzvf ./data/*.tar.gz -C ./data/
purge:
	rm ./data/*.tar.gz
bone010:
	mkdir -p data
	wget -P ./data/ https://sparse.tamu.edu/MM/Oberwolfach/bone010.tar.gz
nlpkkt120:
	mkdir -p data 
	wget -P ./data/ https://sparse.tamu.edu/MM/Schenk/nlpkkt120.tar.gz
nlpkkt240:
	mkdir -p data 
	wget -P ./data/ https://sparse.tamu.edu/MM/Schenk/nlpkkt240.tar.gz

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -DDEBUG -g
debug: all

release: CXXFLAGS += -O2
release: all

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*

info:
	@echo "[*] Application dir: ${APP_DIR}     "
	@echo "[*] Object dir:      ${OBJ_DIR}     "
	@echo "[*] Sources:         ${SRC}         "
	@echo "[*] Objects:         ${OBJECTS}     "
	@echo "[*] Dependencies:    ${DEPENDENCIES}"
