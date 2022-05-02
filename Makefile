all: LoopStrengthReduction.so sub-make

LIB_DIR = ../LIBS
SRC_DIR = src

CXX = clang
CXXFLAGS = $(shell llvm-config --cxxflags) -fcolor-diagnostics -g -O0 -fPIC
OPT = opt
TEST = tests

LoopStrengthReduction.o: $(SRC_DIR)/LoopStrengthReduction.cpp
#IVIdentify.o: $(SRC_DIR)/IVIdentify.cpp

LoopStrengthReduction.so: $(SRC_DIR)/LoopStrengthReduction.o #$(SRC_DIR)/IVIdentify.o 
	$(CXX) -dylib -shared $^ -o $@

sub-make:
	make -C tests

# CLEAN
clean:
	rm -f *.o *~ *.so *.ll $(TEST)/*.bc out* $(SRC_DIR)/*.o             
