SRC		:= src
INC 	:= include
MAIN 	:= $(SRC)/main.c
GCC		:= GCC
FLAGS	:= -I $(INC) -lcrypto -lssl -fopenmp

run: compile
	@chmod +x main.exe
	@echo TEST 1 hash_test_tree
	@./main.exe hash_test_tree d12f92112d97e1454b859f775bed11ed
	@echo TEST 2 pico-sdk-master
	@./main.exe pico-sdk-master c8f154a754ecddaaf00bb270eac18f44
	@echo TEST 3 assignment1_dv1625
	@./main.exe assignment1_dv1625 d4f25ddd40116b69218a2b2836ed1ad5

compile: 
	@gcc $(MAIN) -o main.exe $(FLAGS) 