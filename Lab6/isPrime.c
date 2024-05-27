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
    int id;
    long long int primesCounted;
} ThreadInfo;

Buffer buffer;

long long int sequentialNumOfPrimes = 0;

long long int arraySize = 0;

long long int consumedCount = 0;

sem_t consumedSem;

void bufferInsert(long long int element);

long long int bufferRemove();

void *producer(void *arg);

void *consumer(void *arg);

int ehPrimo(long long int n);

int main(int argc, char* argv[])
{
    ThreadInfo *threadInfos;

    pthread_t threadProd;
    pthread_t *threadsCons;

    int numOfThreads;
    long long int concurrentNumOfPrimes = 0;

    int winnerIndex = 0;
    long long int maxPrimes = 0;

    int i;

    if(argc != 4)
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
        exit(2);
    }
    buffer.in = 0;
    buffer.out = 0;
    sem_init(&buffer.emptySlot, 0, buffer.size);
    sem_init(&buffer.fullSlot, 0, 0);
    sem_init(&buffer.mutexCons, 0, 1);
    sem_init(&consumedSem, 0, 1);

    threadsCons = malloc((sizeof(pthread_t) * numOfThreads));
    if (!threadsCons)
    {
        fprintf(stderr, "Erro de alocação de memória\n");
        exit(2);
    }
    
    threadInfos = malloc(sizeof(ThreadInfo) * numOfThreads);
    if (!threadInfos)
    {
        fprintf(stderr, "Erro de alocação de memória\n");
        exit(2);
    }
    
    if(pthread_create(&threadProd, NULL, producer, (void *) (argv[3])))
    {
        printf("Erro na criacao da thread produtora\n");
        exit(1);
    }
    
    for(i = 0; i < numOfThreads; i++)
    {   
        threadInfos[i].primesCounted = 0;
        if(pthread_create(&threadsCons[i], NULL, consumer, (void *) (&threadInfos[i])))
        {
            printf("Erro na criacao de uma thread consumidora\n");
            exit(1);
        }
    }

    if(pthread_join(threadProd, NULL))
    {
        printf("Erro ao aguardar a thread produtora\n");
        exit(1);
    }

    for(i = 0; i < numOfThreads; i++)
    {
        if(pthread_join(threadsCons[i], NULL))
        {
            printf("Erro ao aguardar uma thread consumidora\n");
            exit(1);
        }
    }

    for(i = 0; i < numOfThreads; i++)
    {
        concurrentNumOfPrimes += threadInfos[i].primesCounted;
        if(threadInfos[i].primesCounted > maxPrimes)
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

    sem_destroy(&consumedSem);
    
    if(sequentialNumOfPrimes == concurrentNumOfPrimes)
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
    char* filePath = (char *) arg;
    FILE *file;
    long long int ret;
    long long int element;
    long long int i = 0;

    file = fopen(filePath, "rb");
    if (!file)
    {
        fprintf(stderr, "Erro de abertura de arquivo %s\n", filePath);
        exit(3);
    }

    ret = fread(&arraySize, sizeof(long long int), 1, file);
    if (!ret)
    {
        fprintf(stderr, "Erro de leitura das dimensões do vetor do arquivo\n");
        exit(3);
    }
    
    while(i < arraySize)
    {
        ret = fread(&element, sizeof(long long int), 1, file);
        if (!ret)
        {
            fprintf(stderr, "Erro de leitura de um dos elementos do vetor\n");
            exit(4);
        }
        bufferInsert(element);
        i++;
    }

    ret = fread(&sequentialNumOfPrimes, sizeof(long long int), 1, file);
    if (!ret)
    {
        fprintf(stderr, "Erro de leitura do número de primos do vetor do arquivo\n");
        exit(3);
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
        sem_wait(&consumedSem);
        if (consumedCount >= arraySize)
        {
            sem_post(&consumedSem);
            break;
        }
        element = bufferRemove();
        consumedCount++;
        sem_post(&consumedSem);

        if(ehPrimo(element))
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