#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    int x;
    if (x)          // ❌ x is uninitialized
        printf("hi\n");
    return 0;
}