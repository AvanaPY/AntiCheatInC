SRC		:= src
INC 	:= include
BIN		:= bin
OUT 	:= $(BIN)/main.exe
MAIN 	:= $(SRC)/main.c
GCC		:= GCC
FLAGS	:= -I $(INC) -lcrypto -lssl -fopenmp

run: clean compile test
	@chmod +x $(OUT)

$(BIN):
	@mkdir -p $(BIN)

compile: $(BIN)
	@echo Compiling...
	@gcc $(MAIN) -o $(OUT) $(FLAGS) 

clean:
	-@rm -rf $(BIN)

test:
	@echo TEST 1 hash_test_tree
	@./$(OUT) test_trees/hash_test_tree d12f92112d97e1454b859f775bed11ed
	@echo TEST 2 pico-sdk-master
	@./$(OUT) test_trees/pico-sdk-master c8f154a754ecddaaf00bb270eac18f44
	@echo TEST 3 assignment1_dv1625
	@./$(OUT) test_trees/assignment1_dv1625 d4f25ddd40116b69218a2b2836ed1ad5
