
make

clang example.c -lm -O0 -Xclang -disable-O0-optnone -o example.out

llvm-dis m2r_example.bc

opt -enable-new-pm=0 --loop-simplify -load ../LoopStrengthReduction.so --lsr ./m2r_example.bc -o opt_example.bc 

llvm-dis opt_example.bc 

llc -filetype=obj opt_example.bc 

clang opt_example.o -lm -O0 -Xclang -disable-O0-optnone -o opt_example.out