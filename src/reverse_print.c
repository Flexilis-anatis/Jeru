#include "reverse_print.h"
#include <string.h>
#include <stdio.h>

void reverse_print(const char *string) {
    size_t index = strlen(string);
    while (index > 0)
        putchar(string[--index]);
}