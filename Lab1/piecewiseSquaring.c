#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Struct representando um vetor
typedef struct
{
    float *value;
    long int size;
} Array;

// Struct representando o argumento a ser passado para cada thread
typedef struct
{
    long int startIndex;
    long int finalIndex;
} ThreadArg;

// Array como variável global para que todas as threads possam acessá-lo
Array *array;

// Aloca espaço de memória para um Array e inicializa seus valores
// com inteiros aleatórios entre 0 e 100
Array *initArray(long int size);
// Cria uma cópia do vetor
Array *copyArray(Array *originalArray);
// Eleva cada elemento do vetor ao quadrado
void *piecewiseSquaring(void *arg);
// Testa se a diferença entre os elementos do vetor original,
// elevados ao quadrado, e dos valores do vetor resultante
// estão abaixo de uma tolerância devido a erros de ponto flutuante
int test(Array *originalArray, Array *resultingArray, float tolerance);
// Imprime o vetor
void printArray(Array *array);

int main(int argc, char *argv[])
{
    srand(time(NULL));

    if (argc == 3)
    {
        printf("Tamanho do vetor : %s\nNúmero de threads : %s\n", argv[1], argv[2]);
    }
    else
    {
        printf("Argumentos não foram fornecidos corretamente.\nDeve haver dois"
        " argumentos, o tamanho do vetor e número de threads.\n");
        return 1;
    }

    long int i;
    long int size = atoi(argv[1]);
    long int elementsPerThread;
    long int finalIndex;
    int NUM_THREADS = atoi(argv[2]);


    pthread_t threads[NUM_THREADS];
    ThreadArg threadArg[NUM_THREADS];

    array = initArray(size);
    Array *testArray = copyArray(array);

    elementsPerThread = ceil((float)array->size / NUM_THREADS);

    printf("\nVetor original :\n\n");
    printArray(array);

    for (i = 0; i < NUM_THREADS; i++)
    {
        // Determina o pedaço do array a ser tratado por cada thread.
        threadArg[i].startIndex = i * elementsPerThread;
        // Toda thread trabalha com o mesmo número de elementos,
        // exceto a última que, caso o tamanho do vetor não seja
        // divisível pelo número de threads, terá um número menor
        // de elementos para tratar
        finalIndex = (i + 1) * elementsPerThread;
        threadArg[i].finalIndex =
        finalIndex < array->size ? finalIndex : array->size;

        if (pthread_create(&threads[i], NULL, piecewiseSquaring, &threadArg[i]))
        {
            printf("--ERRO: pthread_create()\n");
            exit(-1);
        }
    }

    for (i = 0; i < NUM_THREADS; i++)
    {
        if (pthread_join(threads[i], NULL)) {
            printf("--ERRO: pthread_join()\n");
            exit(-2);
        }
    }

    printf("Vetor ao quadrado :\n\n");
    printArray(array);

    if (test(testArray, array, 0.000000000001) == 0)
    {
        printf("O código executou corretamente!\n");
    }
    else
    {
        printf("Houve algum erro no resultado!\n");
    }

    pthread_exit(NULL);

    free(array->value);
    free(array);
    free(testArray->value);
    free(testArray);

    return 0;
}

Array *initArray(long int size)
{
    long int i = 0;
    
    Array *arrayPt = malloc(sizeof(Array));
    arrayPt->value = malloc(sizeof(float) * size);
    arrayPt->size = size;

    for (i = 0; i < size; i++)
    {
        arrayPt->value[i] = (float)rand()/(float)(RAND_MAX/100);
    }

    return arrayPt;
}

Array *copyArray(Array *originalArray)
{
    long int i;
    long int size = originalArray->size;
    
    Array *copyArray = malloc(sizeof(Array));
    copyArray->value = malloc(sizeof(float) * size);
    copyArray->size = size;
    
    for (i = 0; i < size; i++)
    {
        copyArray->value[i] = originalArray->value[i];
    }

    return copyArray;
}

void *piecewiseSquaring(void *arg)
{
    long int i = 0;
    
    ThreadArg *threadArg = (ThreadArg *)arg;

    for (i = 0; i + threadArg->startIndex < threadArg->finalIndex; i++)
        array->value[i + threadArg->startIndex] *= array->value[i + threadArg->startIndex];

    pthread_exit(NULL);
}

int test(Array *originalArray, Array *resultingArray, float tolerance)
{
    long int i;
    int differences = 0;

    for (i = 0; i < originalArray->size; i++)
    {
        if (fabs(originalArray->value[i] * originalArray->value[i]) - resultingArray->value[i] > tolerance)
            differences += 1;
    }

    if (differences != 0)
        return 1;

    return 0;
}

void printArray(Array *array)
{
    long int i;
    
    printf("[");
    for (i = 0; i < array->size - 1; i++)
        printf("%.2f, ", array->value[i]);
    printf("%.2f]\n\n", array->value[array->size - 1]);
}