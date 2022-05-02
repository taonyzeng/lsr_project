#CS5544 Assignment 3: README
#Group: ZENG TAO

/////////////////////////////////
# llvm_assignment3 LICM

To compile the loop invariant code motion pass
# go to the LICM folder
$cd LICM

# run make to compile the LICM pass and the test files under the tests folder. 
$make

# To run the LSR pass
$opt -enable-new-pm=0 --loop-simplify -load ../LoopStrengthReduction.so --sr ./m2r_lsr_bench1.bc -o out_lsr_bench1