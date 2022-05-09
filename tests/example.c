#include <stdio.h>



/*int test3(){
    int k = 1;
    int t = 4;

    for (int i = 5; i < 9; i = i + 1) {
      int j = 3 * i + 2; // <i,3,2> 

      int a = i + k;
      int b = 2 * a;
      t = t + j;
      t = t + b;
    }

    return t;
}*/


int test1(){

    int t = 4;

    for (int i = 5; i < 9; i = i + 1) {
      int j = 3 * i + 2; // <i,3,2> 

      t = t + j;
    }


    return t;
}

/*int test2(){
    int num = 4, j = 0, k = 0, l = 0;
    int a[30] = {1, 1, 1, 1, 1, 1,
                 1, 1, 2, 3, 4, 6,
                 1, 1, 2, 3, 4, 6,
                 1, 1, 2, 3, 4, 6,
                 1, 1, 2, 3, 4, 6};
    printf("%i\n", num + 2);
    printf("%i\n", num + 4);
    for (int i = 0; i < 9; i = i + 1) {
      j = 3 * i + 1; // <i,3,1> 
      a[j] = a[j] - 2;
      i = i + 1;
    }
    printf("%i\n", a[28]);
    int c = 1;
    int b = 2;
    return b;
}*/


int main(int argc, const char** argv) {

  int result = test1();
  printf("t=%i\n", result);

  return result;
}