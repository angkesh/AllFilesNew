#include <stdio.h>

int main() {
    printf("input: \n");
    int a = getchar() - '0';
    int b = getchar() - '0';
    printf("\n%d\n", a);
    printf("\n%d\n", b);

    b *= a;
    printf("\n%d\n", b);
    //a = b / a;
    return 0;
}