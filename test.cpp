#include <stdio.h>
#include <stdlib.h>

int global_var = 0;

void update_global(int *ptr) { *ptr += 1; }

int main() {
  int a = 1, b = 2, c = 0;

  if (a > b) {
    c = a;
  } else {
    c = b;
  }

  for (int i = 0; i < 10; ++i) {
    printf("i = %d\n", i);
    global_var += i;
  }

  int *ptr = &a;
  *ptr = 100;

  update_global(&global_var);

  while (global_var < 20) {
    if (global_var % 2 == 0) {
      b += global_var;
    } else {
      a += global_var;
    }
    global_var++;
  }

  printf("Final: a=%d, b=%d, c=%d\n", a, b, c);

  return a + b + c;
}