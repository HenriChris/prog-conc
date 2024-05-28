#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>

typedef struct
{
    int size;
    int in;
    int out;
    long long int *values;
    sem_t emptySlot, fullSlot;
    sem_t mutexCons;
} Buffer;

typedef struct
{
    long long int primesCounted;
} ThreadInfo;

Buffer buffer;

long long int sequentialNumOfPrimes = 0;
long long int arraySize = 0;

// Insere elementos em um buffer circular
void bufferInsert (long long int element);

//Remove um elemento de um buffer circular
long long int bufferRemove ();

// Carrega valores no buffer a partir de um arquivo binário
void *producer (void *arg);

// Opera sobre os valores do buffer, testando a primalidade deles e contando quantos são primos ou não
void *consumer (void *arg);

// Realiza teste de primalidade
int ehPrimo (long long int n);

int main (int argc, char* argv[])
{
    int numOfThreads;
    
    long long int concurrentNumOfPrimes = 0;
    int winnerIndex = 0;
    long long int maxPrimes = 0;

    ThreadInfo *threadInfos;

    pthread_t threadProd;
    pthread_t *threadsCons;
    
    int i;

    if (argc != 4)
    {
        fprintf(stderr, "Utilização correta : %s "
        "<Quantidade de threads consumidoras> <Tamanho do buffer> <Path do arquivo de entrada>\n", argv[0]);
        return 1;
    }

    numOfThreads = (long long int) atoi(argv[1]);

    buffer.size = (int) atoi(argv[2]);
    buffer.values = malloc((sizeof(long long int)) * buffer.size);
    if (!buffer.values)
    {
        fprintf(stderr, "Erro de alocação de memória\n");
        return 2;
    }
    buffer.in = 0;
    buffer.out = 0;
    sem_init(&buffer.emptySlot, 0, buffer.size);
    sem_init(&buffer.fullSlot, 0, 0);
    sem_init(&buffer.mutexCons, 0, 1);

    threadsCons = malloc((sizeof(pthread_t) * numOfThreads));
    if (!threadsCons)
    {
        fprintf(stderr, "Erro de alocação de memória\n");
        free(buffer.values);
        return 2;
    }
    
    threadInfos = malloc(sizeof(ThreadInfo) * numOfThreads);
    if (!threadInfos)
    {
        fprintf(stderr, "Erro de alocação de memória\n");
        free(buffer.values);
        free(threadsCons);
        return 2;
    }
    
    
    if (pthread_create(&threadProd, NULL, producer, (void *) (argv[3])))
    {
        printf("Erro na criacao da thread produtora\n");
        free(buffer.values);
        free(threadsCons);
        free(threadInfos);

        return 3;
    }
    
    for (i = 0; i < numOfThreads; i++)
    {   
        threadInfos[i].primesCounted = 0;
        if (pthread_create(&threadsCons[i], NULL, consumer, (void *) (&threadInfos[i])))
        {
            printf("Erro na criacao de uma thread consumidora\n");
            free(buffer.values);
            free(threadsCons);
            free(threadInfos);

            return 4;
        }
    }

   if (pthread_join(threadProd, NULL))
    {
        printf("Erro ao aguardar a thread produtora\n");
        free(buffer.values);
        free(threadsCons);
        free(threadInfos);

        return 5;
    }

   for (i = 0; i < numOfThreads; i++)
    {
       if (pthread_join(threadsCons[i], NULL))
        {
            printf("Erro ao aguardar uma thread consumidora\n");
            free(buffer.values);
            free(threadsCons);
            free(threadInfos);
            
            return 6;
        }
    }

   for (i = 0; i < numOfThreads; i++)
    {
        concurrentNumOfPrimes += threadInfos[i].primesCounted;
       if (threadInfos[i].primesCounted > maxPrimes)
        {
            maxPrimes = threadInfos[i].primesCounted;
            winnerIndex = i;
        }
    }

    printf("Número de primos encontrados sequencialmente: %lld\n", sequentialNumOfPrimes);
    printf("Número de primos encontrados concorrentemente: %lld\n", concurrentNumOfPrimes);
    printf("Thread consumidora vencedora: %d (Encontrou %lld primos)\n", winnerIndex + 1, maxPrimes);

    free(buffer.values);
    free(threadsCons);
    free(threadInfos);

    sem_destroy(&buffer.emptySlot);
    sem_destroy(&buffer.fullSlot);
    sem_destroy(&buffer.mutexCons);
    
    if (sequentialNumOfPrimes == concurrentNumOfPrimes)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void bufferInsert(long long int element)
{
    sem_wait(&buffer.emptySlot);
    buffer.values[buffer.in] = element;
    buffer.in = (buffer.in + 1) % buffer.size;
    sem_post(&buffer.fullSlot);
}

long long int bufferRemove()
{
    long long int element;

    sem_wait(&buffer.fullSlot);
    sem_wait(&buffer.mutexCons);
    element = buffer.values[buffer.out];
    buffer.out = (buffer.out + 1) % buffer.size;
    sem_post(&buffer.mutexCons);
    sem_post(&buffer.emptySlot);

    return element;
}

void *producer(void *arg)
{
    char *filePath = (char *) arg;
    FILE *file;
    long long int ret;
    long long int element;
    long long int i = 0;

    file = fopen(filePath, "rb");
    if (!file)
    {
        fprintf(stderr, "Erro de abertura de arquivo %s\n", filePath);
        exit(7);
    }

    ret = fread(&arraySize, sizeof(long long int), 1, file);
    if (!ret)
    {
        fprintf(stderr, "Erro de leitura das dimensões do vetor do arquivo\n");
        exit(8);
    }
    
    while(i < arraySize)
    {
        ret = fread(&element, sizeof(long long int), 1, file);
        if (!ret)
        {
            fprintf(stderr, "Erro de leitura de um dos elementos do vetor\n");
            exit(9);
        }
        bufferInsert(element);
        i++;
    }
    
    ret = fread(&sequentialNumOfPrimes, sizeof(long long int), 1, file);
    if (!ret)
    {
        fprintf(stderr, "Erro de leitura do número de primos do vetor do arquivo\n");
        exit(10);
    }

    for (i = 0; i < buffer.size; i++)
    {
        // Valor para indicar que o produtor já terminou de produzir
        bufferInsert(-1);
    }

    fclose(file);
    pthread_exit(NULL);
}

void *consumer(void *arg)
{
    long long int element;
    ThreadInfo *thread = (ThreadInfo *) arg;

    while(1)
    {
        element = bufferRemove();
        if (element == -1)
        {
            // Devolve o -1 para que outras threads consumidoras saibam que o produtor terminou de produzir
            bufferInsert(-1);
            break;
        }

        if (ehPrimo(element))
        {
            thread->primesCounted += 1;
        }
    }
    
    pthread_exit(NULL);
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