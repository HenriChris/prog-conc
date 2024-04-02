#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void outputArray(long int size);

int main(int argc, char* argv[])
{
    long int arraySize;
    if(argc != 2)
    {
        printf("Utilização correta: %s <tamanho_vetor>\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    arraySize = (long int)atoi(argv[1]);

    if(arraySize <= 0)
    {
        printf("Tamanho do vetor deve ser um inteiro positivo.\n");
        return 2;
    }

    outputArray(arraySize);
    printf("\n");

    return 0;
}

void outputArray(long int size)
{
    long int i;
    printf("%ld ", size);
    for(i = 0; i < size; i++)
        printf("%f ", (float)rand() * 100 / RAND_MAX);
}
