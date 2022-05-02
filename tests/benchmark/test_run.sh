for f in $(find . -type f -name *.[c\h])
do
  filename="${f%.*}"
  
  echo ${filename}

  #clang ${f} -O0 -Xclang -disable-O0-optnone -o ${filename}.out
done