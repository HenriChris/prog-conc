#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

typedef struct
{
    float* value;
    long int size;
} Array;

typedef struct
{
    long int startIndex, finalIndex;
} ThreadArg;

Array* array;

Array* loadArray();
void* sumArray(void* arg);
int test(Array* array, float arraySum, float errorTolerance);
void printArray(Array* array);

int main(int argc, char* argv[])
{
    long int i, blockSize;
    int NUM_THREADS;
    pthread_t* threads;
    ThreadArg** threadargs;
    float* retorno;
    float sum = 0.0;

    if(argc != 2)
    {
        printf("Erro no argumento de linha de comando\n");
        return 1;
    }

    NUM_THREADS = atoi(argv[1]);
    threadargs = malloc(sizeof(ThreadArg*) * NUM_THREADS);
    if(threadargs == NULL)
    {
        fprintf(stderr, "ERRO--malloc\n");
        return 2;
    }

    array = loadArray();

    printf("Vetor: \n\n");
    printArray(array);

    threads = (pthread_t *) malloc(sizeof(pthread_t) * NUM_THREADS);
    if(threads == NULL)
    {
        fprintf(stderr, "ERRO--malloc\n");
        return 2;
    }
    blockSize = ceil(array->size / NUM_THREADS);
    if(!blockSize) 
        printf("\nA quantidade de threads é maior que a quantidade de elementos, a execução será sequencial!\n");

    for(i = 0; i < NUM_THREADS; i++)
    {
        threadargs[i] = malloc(sizeof(ThreadArg));
        if(threadargs[i] == NULL)
        {
            fprintf(stderr, "ERRO--malloc\n");
            return 2;
        }
        threadargs[i]->startIndex = i * blockSize;
        threadargs[i]->finalIndex = threadargs[i]->startIndex + blockSize - 1;
        
        if(pthread_create(threads + i, NULL, sumArray, (void*) threadargs[i]))
        {
            fprintf(stderr, "ERRO--pthread_create\n");
            return 3;
        }
    }

    for(i = 0; i < NUM_THREADS; i++)
    {
        if(pthread_join(*(threads + i), (void**) &retorno)){
            fprintf(stderr, "ERRO--pthread_create\n");
            return 4;
        }
        sum += *retorno;
    }

    printf("\n\nSoma de todos os valores do vetor: %f\n", sum);

    if(test(array, sum, 0.0001))
    {
        printf("O teste falhou.\n");
    }
    else
    {
        printf("O teste foi um sucesso.\n");
    }

    free(array);
    free(threadargs);
    return 0;
}

Array* loadArray()
{
    long int i, size;

    Array* array;

    scanf("%ld", &size);
    array = malloc(sizeof(Array));
    if(array == NULL)
    {
        fprintf(stderr, "ERRO--malloc\n");
    }
    
    array->size = size;
    array->value = malloc(sizeof(float) * size);
    if(array->value == NULL)
    {
        fprintf(stderr, "ERRO--malloc\n");
    }

    for(i = 0; i < size; i++)
        scanf("%f", &array->value[i]);

    return array;
}

void* sumArray(void* arg)
{
    ThreadArg *args = (ThreadArg *) arg;
    int i;
    float* sum;
    sum = malloc(sizeof(float));
    *sum = 0.0;

    for(i = args->startIndex; i <= args->finalIndex && i < array->size; i++)
        *sum += array->value[i];

    pthread_exit((void *) sum); 
}

int test(Array* array, float arraySum, float errorTolerance)
{
    int i;
    for(i = 0; i < array->size; i++)    
        arraySum -= array->value[i];

    if(arraySum > errorTolerance)
        return 1;

    return 0;
}

void printArray(Array* array)
{
    int i;
    printf("[");
    for(i = 0; i < array->size - 1; i++)
        printf("%f, ", array->value[i]);
    printf("%f]\n", array->value[array->size - 1]);
}