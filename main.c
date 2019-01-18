#include "src/jerutype.h"

int main(void) {
    JeruType *five = init_jeru_int(5);
    print_jeru_type(five);

    return 0;
}