#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


int main(){
    printf("Hello");
    printf("\n");
    fork();
    printf("BYE");
    return 0;
}