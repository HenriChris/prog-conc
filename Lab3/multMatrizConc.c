#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "matriz.h"
#include "timer.h"

// Struct do argumento de cada thread
typedef struct
{
    // As matrizes serão divididas em blocos. No caso da C, cada bloco será acessado por uma
    // e apenas uma thread. Na A e na B, blocos poderão ser acessados por múltiplas threads,
    // mas, como não serão sobrescritas após a inicialização, não haverá problemas.
    Matrix* A;
    Matrix* B;
    Matrix* C;
    // Coordenadas do primeiro elemento do bloco de C que será operado pela thread
    long int startLine, startColumn;
    // Tamanho do bloco de C que será operado pela thread
    long int n, m;
} ThreadArg;

int main (int argc, char* argv[])
{   
    // timer
    double start, finish, elapsed;
    
    Matrix A;
    Matrix B;
    Matrix C;

    int i, NUM_THREADS;
    long int size;

    pthread_t* threads;
    ThreadArg** threadargs;

    if (argc != 5)
    {
        fprintf(stderr, "Utilização correta :"
        "%s <Path do arquivo de entrada de A> <Path do arquivo de entrada de B>"
        " <Path do arquivo de saída> <Quantidade de threads>\n", argv[0]);
        return 1;
    }

    A = inputMatrix(argv[1]);
    B = inputMatrix(argv[2]);
    if (A.m != B.n)
    {
        fprintf(stderr, "Número de colunas de A (%ld) deve ser igual ao número de linhas de B (%ld)\n", A.m, B.n);
        return 2;
    }

    size = A.n * B.m;
    C.value = malloc(sizeof(float) * size);
    if(!C.value)
    {
        fprintf(stderr, "Erro de alocação de memória\n");
        return 3;
    }
    C.n = A.n;
    C.m = B.m;

    NUM_THREADS = atoi(argv[4]);
    if(NUM_THREADS <= 0)
    {
        fprintf(stderr, "Número de threads deve ser um inteiro positivo.\n");
        return 4;
    }
    
    threadargs = malloc(sizeof(ThreadArg*) * NUM_THREADS);
    if(!threadargs)
    {
        fprintf(stderr, "Erro de alocação de memória\n");
        return 5;
    }

    threads = malloc(sizeof(pthread_t) * NUM_THREADS);
    if(!threads)
    {
        fprintf(stderr, "Erro de alocação de memória\n");
        return 5;
    }

    GET_TIME(start);
    for(i = 0; i < NUM_THREADS; i++)
    {
        threadargs[i] = malloc(sizeof(ThreadArg));
        if(!threadargs[i])
        {
            fprintf(stderr, "Erro de alocação de memória\n");
            return 5;
        }

        threadargs[i]->A = &A;
        threadargs[i]->B = &B;
        threadargs[i]->C = &C;

        // Se o número de linhas for maior que o número de colunas, divide a matriz por linhas,
        // caso contrário, por colunas
        if (C.n > C.m)
        {
            if (NUM_THREADS > C.n) NUM_THREADS = C.n;
            threadargs[i]->n = C.n / NUM_THREADS;
            threadargs[i]->m = C.m;
            threadargs[i]->startLine = i * (C.n / NUM_THREADS);
            threadargs[i]->startColumn = 0;
        }
        else
        {
            if (NUM_THREADS > C.m) NUM_THREADS = C.m;
            threadargs[i]->n = C.n;
            threadargs[i]->m = C.m / NUM_THREADS;
            threadargs[i]->startLine = 0;
            threadargs[i]->startColumn = i * (C.m / NUM_THREADS);
            printf("\n\n Tamanho : %ld x %ld \n Coordenadas do primeiro elemento : (%ld, %ld)\n", threadargs[i]->n, threadargs[i]->m, threadargs[i]->startLine, threadargs[i]->startColumn);
        }

        if(pthread_create(&threads[i], NULL, multMatrixConc, (void*)threadargs[i]))
        {
            fprintf(stderr, "Erro de criação de thread\n");
            return 4;
        }
    }

    for(i = 0; i < NUM_THREADS; i++)
    {
        if (pthread_join(threads[i], NULL))
        {
            fprintf(stderr, "Erro de junção de thread\n");
            return 5;
        }

        free(threadargs[i]);
    }

    GET_TIME(finish);
    elapsed = finish - start;

    outputMatrix(C, argv[3]);
    
    //printf("%e\n", elapsed);

    free(A.value);
    free(B.value);
    free(C.value);

    free(threads);

    return 0;
}

Matrix inputMatrix(char* filePath)
{
    long int size;
    Matrix matrix;
    FILE* file;
    long int ret;

    file = fopen(filePath, "rb");
    if (!file)
    {
        fprintf(stderr, "Erro de abertura de arquivo %s\n", filePath);
        exit(2);
    }

    ret = fread(&matrix.n, sizeof(long int), 1, file);
    if (!ret)
    {
        fprintf(stderr, "Erro de leitura das dimensões da matriz do arquivo\n");
        exit(3);
    }
    ret = fread(&matrix.m, sizeof(long int), 1, file);
    if (!ret)
    {
        fprintf(stderr, "Erro de leitura das dimensões da matriz do arquivo\n");
        exit(3);
    }

    size = matrix.n * matrix.m;
    matrix.value = malloc(sizeof(float) * size);
    if (!matrix.value)
    {
        fprintf(stderr, "Erro de alocação de memória\n");
        exit(4);
    }
    ret = fread(matrix.value, sizeof(float), size, file);
    if(ret < size)
    {
        fprintf(stderr, "Erro de leitura dos elementos da matriz\n");
        exit(4);
    }

    fclose(file);
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

void* multMatrixConc(void* args)
{
    ThreadArg* arg = (ThreadArg*)args;

    long int i, j, k;

    // Opera apenas nas linhas ou colunas associadas a sua thread
    for (i = arg->startLine; i < arg->startLine + arg->n; i++)
    {
        for (j = arg->startColumn; j < arg->startColumn + arg->m; j++)
        {
            arg->C->value[i + j * arg->C->n] = 0.0; // C[i][j] = 0.0 Inicializa o valor como 0.0 por garantia
            for (k = 0; k < arg->A->m; k++)
            {
                // C[i][j] += A[i][k] * B[k][j]
                arg->C->value[i + j * arg->C->n] += arg->A->value[i + k * arg->A->n] * arg->B->value[k + j * arg->B->n];
            }
        }
    }

    pthread_exit(NULL);
}