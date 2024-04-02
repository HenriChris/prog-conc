#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void outputArray(long int size);

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        printf("Erro no argumento de linha de comando\n");
        return 1;
    }

    srand(time(NULL));

    outputArray(atoi(argv[1]));

    return 0;
}

void outputArray(long int size)
{
    int i;
    printf("%ld ", size);
    for(i = 0; i < size; i++)
        printf("%f ", (float)rand()/(float)(RAND_MAX/100));
}