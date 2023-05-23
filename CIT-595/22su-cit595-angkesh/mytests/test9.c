#include <stdio.h>

int main() {
  int sum = 0, len = 0;
  int inp;

  while ((inp = getchar()) && len < 20) {
    printf("\ninp: %d\n", inp);
    sum += inp;
    len++;
  }

  int avg = sum / len;

  return 0;
}