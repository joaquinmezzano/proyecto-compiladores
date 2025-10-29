#include <stdio.h>

int get_int() {
    long long x;
    printf("Enter number: ");
    scanf("%lld", &x);
    return x;
}

void print_int(long long x) {
    printf("Number: %lld\n", x);
}