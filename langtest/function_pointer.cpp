// Example program
#include <iostream>
#include <string>

typedef int (*F)(int i);

int ff(int i)
{
    printf("ff %d\n", i);
    return i;
}

int fff(int i)
{
    printf("fff %d\n", i);
    return i;    
}

void run(F func, int i)
{
    static F f;
    switch (i) {
    case 1:
        f = func;
        printf("%p\n", f);
        if( f == (*f) )
            (*f)(5);
        break;
    case 2:
        f(999);
        break;
    }
}

int main()
{
    run(ff, 1);
    run(fff, 2);
    run(fff, 1);
    run(ff, 2);
}