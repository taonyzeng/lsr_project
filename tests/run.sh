
make

clang example.c -lm -O0 -Xclang -disable-O0-optnone -o example.out

llvm-dis m2r_example.bc

opt -enable-new-pm=0 --loop-simplify -load ../LoopStrengthReduction.so --lsr ./m2r_example.bc -o opt_example.bc 

llvm-dis opt_example.bc 


opt -enable-new-pm=0  -load ../IndvarElimination.so --indvar-el ./opt_example.bc -o indvar_eli_example.bc

llvm-dis indvar_eli_example.bc


opt -enable-new-pm=0 --loop-simplify --loop-reduce ./m2r_example.bc -o llvm_opt_example.bc

llvm-dis llvm_opt_example.bc


llc -filetype=obj opt_example.bc 
clang opt_example.o -lm -O0 -Xclang -disable-O0-optnone -o opt_example.out


llc -filetype=obj indvar_eli_example.bc
clang indvar_eli_example.o -lm -O0 -Xclang -disable-O0-optnone -o indvar_eli_example.out