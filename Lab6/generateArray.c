#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "array.h"

Array generateArray(long long int size);

void saveArray(Array array, char* filePath);

int main (int argc, char* argv[])
{
    long long int size;
    Array array;

    if (argc != 3)
    {
        fprintf(stderr, "Utilização correta : %s "
        "<Número de elementos> <Path do arquivo a ser gerado>\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    size = (long long int) atoi(argv[1]);

    array = generateArray(size);

    saveArray(array, "./array");

    free(array.values);

    return 0;
}

int ehPrimo(long long int n)
{
    int i;
    if (n <= 1)
        return 0;
    if (n == 2)
        return 1;
    if (n % 2 == 0)
        return 0;
    for (i = 3; i < sqrt(n) + 1; i += 2)
        if (n % i == 0)
            return 0;
    return 1;
}

Array generateArray(long long int size)
{
    long long int i;
    Array array;

    array.numberOfPrimes = 0;
    array.size = size;
    array.values = malloc(size * sizeof(long long int));
    if (!array.values)
    {
        fprintf(stderr, "Erro de alocação de memória\n");
        exit(2);
    }

    for(i = 0; i < size; i++)
        array.values[i] = rand(); // TODO : como balancear os valores?

    for(i = 0; i < array.size; i++)
    {
        if(ehPrimo(array.values[i]))
            array.numberOfPrimes += 1;
    }

    return array;
}

void saveArray(Array array, char* filePath)
{
    FILE* file;
    long long int ret;

    file = fopen(filePath, "wb");
    if(!file)
    {
        fprintf(stderr, "Erro de abertura do arquivo\n");
        exit(3);
    }

    ret = fwrite(&array.size, sizeof(long long int), 1, file);

    ret = fwrite(array.values, sizeof(long long int), array.size, file);
    if(ret < array.size)
    {
        fprintf(stderr, "Erro de escrita no arquivo\n");
        exit(4);
    }

    ret = fwrite(&array.numberOfPrimes, sizeof(long long int), 1, file);

    fclose(file);
}