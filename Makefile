EXE = osProj2
CC = gcc
CXX = g++
LD = g++
CFLAGS = 
CXXFLAGS = 
CPPFLAGS = -Wall
DEPFLAGS = -MMD -MP
LDFLAGS = 
LDLIBS = 
BIN = bin
OBJ = obj
SRC = src
SOURCES := $(wildcard $(SRC)/*.c $(SRC)/*.cc $(SRC)/*.cpp $(SRC)/*.cxx)
OBJECTS := \
	$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(wildcard $(SRC)/*.c)) \
	$(patsubst $(SRC)/%.cc, $(OBJ)/%.o, $(wildcard $(SRC)/*.cc)) \
	$(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(wildcard $(SRC)/*.cpp)) \
	$(patsubst $(SRC)/%.cxx, $(OBJ)/%.o, $(wildcard $(SRC)/*.cxx))
DEPENDS := $(OBJECTS:.o=.d)
COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o $@
COMPILE.cxx = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c -o $@
LINK.o = $(LD) $(LDFLAGS) $(LDLIBS) $(OBJECTS) -o $@
.DEFAULT_GOAL = all
.PHONY: all
all: $(BIN)/$(EXE)
$(BIN)/$(EXE): $(SRC) $(OBJ) $(BIN) $(OBJECTS)
	$(LINK.o)
$(SRC):
	mkdir -p $(SRC)
$(OBJ):
	mkdir -p $(OBJ)
$(BIN):
	mkdir -p $(BIN)
$(OBJ)/%.o: $(SRC)/%.c
	$(COMPILE.c) $<
$(OBJ)/%.o: $(SRC)/%.cc
	$(COMPILE.cxx) $<
$(OBJ)/%.o: $(SRC)/%.cpp
	$(COMPILE.cxx) $<
$(OBJ)/%.o: $(SRC)/%.cxx
	$(COMPILE.cxx) $<
.PHONY: remake
remake: clean $(BIN)/$(EXE)
.PHONY: run
run: $(BIN)/$(EXE)
	./$(BIN)/$(EXE)
.PHONY: clean
clean:
	$(RM) $(OBJECTS)
	$(RM) $(DEPENDS)
	$(RM) $(BIN)/$(EXE)
.PHONY: reset
reset:
	$(RM) -r $(OBJ)
	$(RM) -r $(BIN)
-include $(DEPENDS)
