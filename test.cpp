#include <stdio.h>

int main() {
    // int n, i;
    // int a = 0, b = 1, next;

    // printf("Enter the number of Fibonacci terms to display: ");
    // scanf("%d", &n);

    // if (n <= 0) {
    //     printf("Please enter a positive integer.\n");
    //     return 1;
    // }

    // printf("Fibonacci Series: ");

    // for (i = 0; i < n; i++) {
    //     printf("%d ", a);
    //     next = a + b;
    //     a = b;
    //     b = next;
    // }

    // printf("\n");
    // return 0;
    int n;
    scanf("%d", &n);
    for (int i = 1; i < 100; ++i) {
      printf("%d %d %d\n", i, n, i * n);
    }
    return n;
}
