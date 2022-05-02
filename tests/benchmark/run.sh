
for f in *.c
do
  filename="${f%.*}"
  clang ${f} ./support/beebsc.c -lm -O0 -Xclang -disable-O0-optnone -I./support -o ${filename}.out
done

echo '===============original compile finished========================'

for f in *.bc
do
  # optimze with llvm opt
  #opt -enable-new-pm=0 -S -load build/skeleton/libSkeletonPass.so -mem2reg --sr ${f} -o opt_${f}.bc
  filename="${f%.*}"

  opt -enable-new-pm=0 --loop-simplify -load ../../LoopStrengthReduction.so --sr ${f} -o opt_${filename}.bc
  llc -filetype=obj opt_${filename}.bc 
  #gcc opt_${filename}.o -lm -o opt_${filename}.out -fPIE
  clang opt_${filename}.o ./support/beebsc.c -lm -O0 -Xclang -disable-O0-optnone -I./support -o opt_${filename}.out
done

echo '===============opt compile finished========================'


for f in *.c
do
  filename="${f%.*}"
  opt_out=opt_m2r_${filename}.out
  orignal_out=${filename}.out

  hyperfine  './'${opt_out}  './'${orignal_out} -i --export-markdown ${filename}_report.md
done