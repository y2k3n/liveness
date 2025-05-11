#include <stdio.h>

int getn();
void scann(int i);

int main() {
  int n;
  scanf("%d", &n);
  printf("%d\n", n);
  int j = getn();
  scann(j);
}

int getn() {
  int n;
  scanf("%d", &n);
  printf("%d\n", n);
  return n;
}

void scann(int i) {
  int n;
  scanf("%d", &n);
  printf("%d\n", i*n);
}
