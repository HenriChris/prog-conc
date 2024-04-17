#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "matriz.h"

int main (int argc, char* argv[])
{
    Matrix matrix;
    
    if (argc != 4)
    {
        fprintf(stderr, "Utilização correta : %s"
        "<Número de linhas> <Número de colunas> <Path do arquivo a ser gerado>", argv[0]);
        return 1;
    }

    srand(time(NULL));
    matrix = initMatrix((long int) atoi(argv[1]), (long int) atoi(argv[2]));
    outputMatrix(matrix, argv[3]);
    free(matrix.value);
    return 0;
}

Matrix initMatrix(long int n, long int m)
{
    long int i, size;
    Matrix matrix;

    size = n * m;
    
    matrix.n = n;
    matrix.m = m;
    matrix.value = malloc(sizeof(float) * size);
    if (!matrix.value)
    {
        fprintf(stderr, "Erro de alocação de memória\n");
        exit(2);
    }

    for (i = 0; i < size; i++)
    {
        matrix.value[i] = (float) rand() * 1000 / RAND_MAX;
    }

    return matrix;
}

void outputMatrix(Matrix matrix, char* filePath)
{
    FILE* file;
    long int ret;
    long int size = matrix.n * matrix.m;
    
    file = fopen(filePath, "wb");
    if(!file)
    {
        fprintf(stderr, "Erro de abertura do arquivo\n");
        exit(3);
    }

    ret = fwrite(&matrix.n, sizeof(long int), 1, file);
    ret = fwrite(&matrix.m, sizeof(long int), 1, file);

    ret = fwrite(matrix.value, sizeof(float), matrix.n * matrix.m, file);

    if(ret < size)
    {
        fprintf(stderr, "Erro de escrita no arquivo\n");
        exit(4);
    }

    fclose(file);
}