all: LoopStrengthReduction.so IndvarElimination.so sub-make

LIB_DIR = ../LIBS
SRC_DIR = src

CXX = clang
CXXFLAGS = $(shell llvm-config --cxxflags) -fcolor-diagnostics -g -O0 -fPIC
OPT = opt
TEST = tests

LoopStrengthReduction.o: $(SRC_DIR)/LoopStrengthReduction.cpp
#IVIdentify.o: $(SRC_DIR)/IVIdentify.cpp

IndvarElimination.o: $(SRC_DIR)/IndvarElimination.cpp

LoopStrengthReduction.so: $(SRC_DIR)/LoopStrengthReduction.o #$(SRC_DIR)/IVIdentify.o 
	$(CXX) -dylib -shared $^ -o $@

IndvarElimination.so: $(SRC_DIR)/IndvarElimination.o #$(SRC_DIR)/IVIdentify.o 
	$(CXX) -dylib -shared $^ -o $@

sub-make:
	make -C tests

# CLEAN
clean:
	rm -f *.o *~ *.so *.ll $(TEST)/*.bc out* $(SRC_DIR)/*.o             
