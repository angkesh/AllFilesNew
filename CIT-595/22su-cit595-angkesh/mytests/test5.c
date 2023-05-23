#include <stdio.h>
#include <string.h>

int main() {
    int a = 0;
    char s[20];
    fgets(s, sizeof(s), stdin);
    int l = 0;
    int r = strnlen(s, 20) - 1;
    printf("\nr: %d\n", r);
    // printf("\ns: %d\n", s[r]);
    if (r < 5) {
        printf("\nhere1\n");
        a = 1;
    }

    while (l < r) {

        if (s[l++] != s[r--]) {
            a = 1;
            printf("\nhere2\n");
        }
    }

    if (s[2] != s[1] + 4) {
        a = 1;
        printf("\nhere3\n");
    }
    printf("\n%d\n", a);
    int b = 1 / a;

    return 0;
}
