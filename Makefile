GCC		:= gcc
BIN		:= ./bin
OBJ		:= ./obj
INCLUDE	:= ./include
SRC		:= ./src
SRCS	:= $(wildcard $(SRC)/*.c)
OBJS    := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))
EXE		:= $(BIN)/main.exe
FLAGS	:= -fopenmp -O3
LIBS	:= -lcrypto

.PHONY: all run clean

all: $(EXE)

$(EXE): $(OBJS) | $(BIN)
	$(GCC) -I $(INCLUDE) $(FLAGS) $^ -o $@ $(LIBS)

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(GCC) -I $(INCLUDE) $(FLAGS) $(LIBS) -c $< -o $@

$(BIN) $(OBJ):
	@mkdir $@

run: $(EXE) test

clean:
	-@rm -rf $(OBJ)
	-@rm -rf $(BIN)

test: $(EXE)
	@echo TEST 1 hash_test_tree
	@./$(BIN)/main.exe test_trees/hash_test_tree d12f92112d97e1454b859f775bed11ed
	@echo TEST 2 pico-sdk-master
	@./$(BIN)/main.exe test_trees/pico-sdk-master c8f154a754ecddaaf00bb270eac18f44
	@echo TEST 3 assignment1_dv1625
	@./$(BIN)/main.exe test_trees/assignment1_dv1625 d4f25ddd40116b69218a2b2836ed1ad5