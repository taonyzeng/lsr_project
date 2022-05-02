
for f in *.c
do
  filename="${f%.*}"
  clang ${f} -O0 -Xclang -disable-O0-optnone -o ${filename}.out
done

for f in *.bc
do
  # optimze with llvm opt
  #opt -enable-new-pm=0 -S -load build/skeleton/libSkeletonPass.so -mem2reg --sr ${f} -o opt_${f}.bc
  filename="${f%.*}"

  opt -enable-new-pm=0 --loop-simplify -load ../LoopStrengthReduction.so --sr ${f} -o opt_${filename}.bc
  llc -filetype=obj opt_${filename}.bc 
  #gcc opt_${filename}.o -lm -o opt_${filename}.out -fPIE
  clang opt_${filename}.o -O0 -Xclang -disable-O0-optnone -o opt_${filename}.out
done