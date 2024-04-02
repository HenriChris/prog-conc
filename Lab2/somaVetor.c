#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

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
long int min(long int a, long int b);
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
        printf("Utilização correta: %s <num_threads>\n", argv[0]);
        return 1;
    }

    NUM_THREADS = atoi(argv[1]);
    if(NUM_THREADS <= 0)
    {
        printf("Número de threads deve ser um inteiro positivo.\n");
        return 2;
    }

    threadargs = malloc(sizeof(ThreadArg*) * NUM_THREADS);
    if(threadargs == NULL)
    {
        fprintf(stderr, "ERRO--malloc\n");
        return 3;
    }

    array = loadArray();

    printf("Vetor gerado: \n\n");
    printArray(array);

    threads = malloc(sizeof(pthread_t) * NUM_THREADS);
    if(threads == NULL)
    {
        fprintf(stderr, "ERRO--malloc\n");
        return 3;
    }
    
    blockSize = array->size / NUM_THREADS;
    if(blockSize <= 1)
    {
        printf("\nNúmero de threads igual a ou maior que número de elementos no vetor.\n");
        blockSize = 1;
    }

    for(i = 0; i < NUM_THREADS; i++)
    {
        threadargs[i] = malloc(sizeof(ThreadArg));
        if(threadargs[i] == NULL)
        {
            fprintf(stderr, "ERRO--malloc\n");
            return 3;
        }

        threadargs[i]->startIndex = i * blockSize;
        threadargs[i]->finalIndex = min((i + 1) * blockSize, array->size) - 1;
        
        if(pthread_create(&threads[i], NULL, sumArray, (void*)threadargs[i]))
        {
            fprintf(stderr, "ERRO--pthread_create\n");
            return 4;
        }
    }

    for(i = 0; i < NUM_THREADS; i++)
    {
        if (pthread_join(threads[i], (void**) &retorno))
        {
            fprintf(stderr, "ERRO--pthread_create\n");
            return 5;
        }
        sum += *retorno;
        free(retorno);
        free(threadargs[i]);
    }

    printf("\nSoma de todos os elementos do array: %f\n", sum);

    if(test(array, sum, 0.0001))
    {
        printf("O teste falhou.\n");
    }
    else
    {
        printf("O teste foi um sucesso.\n");
    }

    free(array->value);
    free(array);
    free(threadargs);
    free(threads);

    return 0;
}

Array* loadArray()
{
    long int i, size;

    Array* array;

    if(scanf("%ld", &size) != 1 || size <= 0)
    {
        printf("Valor inválido para tamanho do vetor.\n");
        exit(6);
    }

    array = malloc(sizeof(Array));
    if(array == NULL)
    {
        fprintf(stderr, "ERRO--malloc\n");
        exit(3);
    }
    
    array->size = size;
    array->value = malloc(sizeof(float) * size);
    if(array->value == NULL)
    {
        fprintf(stderr, "ERRO--malloc\n");
        exit(3);
    }
    
    for(i = 0; i < size; i++)
    {
        if (scanf("%f", &array->value[i]) != 1) {
            printf("Valor inválido para elemento do vetor.\n");
            exit(7);
        }
    }

    return array;
}

void* sumArray(void* arg)
{
    ThreadArg* args = (ThreadArg*) arg;
    int i;
    float* sum;

    sum = malloc(sizeof(float));
    if(sum == NULL)
    {
        fprintf(stderr, "ERRO--malloc\n");
        pthread_exit(NULL);
    }

    *sum = 0.0;

    for(i = args->startIndex; i <= args->finalIndex; i++)    
        *sum += array->value[i];

    pthread_exit((void*) sum);
}

long int min(long int a, long int b)
{    
    if(a < b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

int test(Array* array, float arraySum, float errorTolerance)
{
    long int i;
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