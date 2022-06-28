#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEBUG

int IsPassOk(void);

int main(void)
{
    int PwStatus;

#ifdef DEBUG
printf("&main = %p, &IsPassOk = %p\n\n", &main, &IsPassOk);
#endif

    puts("Enter password:");
    PwStatus = IsPassOk();

    if (PwStatus == 0) {
        printf("Bad password!\n");
        exit(1);
    } else {
        printf("Access granted!\n");
    }

    return 0;
}

int IsPassOk(void)
{
    char Pass[12];
    
    gets(Pass);

    return 0 == strcmp(Pass, "test");
}