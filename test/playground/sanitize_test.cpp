#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    int x;
    if (x) // ❌ x is uninitialized
        printf("hi\n");
    return 0;
}