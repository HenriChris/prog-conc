#ifndef MATRIZ_H
#define MATRIZ_H

// Struct representando uma matriz utlizando um vetor de floats
typedef struct
{
    float* value;
    long int n, m;
} Matrix;

// Gera um struct de matriz com n * m elementos com valores entre 0 e 1000
Matrix initMatrix(long int n, long int m);
// Multiplica duas matrizes sequencialmente
Matrix multMatrixSeq(Matrix A, Matrix B);
// Multiplica duas matrizes concorrentemente
void* multMatrixConc(void* args);
// Carrega uma matriz a partir de um arquivo binário
Matrix inputMatrix(char* filePath);
// Salva uma matriz em um arquivo binário
void outputMatrix(Matrix matrix, char* filePath);

#endif