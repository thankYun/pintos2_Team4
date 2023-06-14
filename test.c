#include <stdio.h>

int main()
{asdfsafkn;adfna;nf;
    long num = 4613937818241073152;
    long* ptr = &num;
    long** dptr = &ptr;
    long*** tptr = &dptr;

    printf("\n abcd~~ abcd~~~ 0x%02X \n", *(long **)dptr);
    printf("\n abcd~~ abcd~~~ 0x%02X \n", **(long **)dptr);
    printf("\n abcd~~ abcd~~~ 0x%02X \n", **(long ***)dptr);
    printf("\n abcd~~ abcd~~~ 0x%02X \n", ***(long ***)dptr);

    //int aList[10] = {1,2,3,4,5,6,7,8,9,10};
    return 0;
}
