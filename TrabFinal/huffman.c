#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>

#define MAX_TREE_HT 256
#define NUM_CHARS 256
#define NUM_THREADS 4

// Variáveis globais
sem_t slotCheio, slotVazio;  // semáforos para sincronização por condição
sem_t mutexGeral; //semaforo UNICO para sincronização entre produtores e consumidores e para log

int N;           // número de consumidores
int M;           // tamanho do buffer
int B;           // tamanho do bloco processado
FILE *file;      // arquivo texto

char **Buffer;

int terminou = 0; // flag para indicar que a produção terminou

// função para inserir um elemento no buffer
void Insere(char* item) {
    static int in = 0;
    sem_wait(&slotVazio); // aguarda slot vazio para inserir
    sem_wait(&mutexGeral); // exclusão mútua entre produtores (aqui geral para log)

    Buffer[in] = item;
    in = (in + 1) % M;
    sem_post(&mutexGeral);
    sem_post(&slotCheio); // sinaliza um slot cheio
}

// função para retirar um elemento no buffer
char *Retira() {
    char *item;
    static int out = 0;
    sem_wait(&slotCheio); // aguarda slot cheio para retirar
    sem_wait(&mutexGeral); // exclusão mútua entre consumidores (aqui geral para log)
    item = Buffer[out];
    Buffer[out] = NULL;
    out = (out + 1) % M;
    sem_post(&mutexGeral);
    sem_post(&slotVazio); // sinaliza um slot vazio
    return item;
}

// função que testa se o buffer tem algum item não processado para condição de parada das threads consumidoras
int bufferCheio(char **vetor, int tamanho) {
    for (int i = 0; i < tamanho; i++) {
        if (vetor[i] != NULL) {
            return 1; // retorna 1 se encontrar um elemento diferente de NULL
        }
    }
    return 0; // retorna 0 se todos os elementos forem NULL
}

void *produtor(void *arg) {
    while (1) {
        char *bloco = (char *)malloc(B * sizeof(char));
        if (fgets(bloco, B, file) == NULL) {
            free(bloco);
            break;
        }
        Insere(bloco);
    }

    sem_wait(&mutexGeral);
    terminou = 1;
    sem_post(&mutexGeral);
    for (int i = 0; i < N; i++) {
        sem_post(&slotCheio); 
    }
    pthread_exit(NULL);
}

void *consumidor(void *arg) {
    long long int *counts = calloc(NUM_CHARS, sizeof(long long int));
    if (counts == NULL) {
        fprintf(stdout, "Erro de alocação de memória.\n");
        exit(-2);
    }

    while (1) {
        sem_wait(&mutexGeral);
        if (terminou && !bufferCheio(Buffer, M)) {
            sem_post(&mutexGeral);
            break;
        }
        sem_post(&mutexGeral);
        char *item = Retira();
        if (item != NULL) {
            for (int i = 0; item[i] != '\0'; i++) {
                counts[(unsigned char)item[i]]++;
            }
            free(item);
        }
    }

    return (void *)counts;
}

struct MinHeapNode { 
    char data; 
    unsigned freq; 
    struct MinHeapNode *left, *right; 
}; 

struct MinHeap { 
    unsigned size; 
    unsigned capacity; 
    struct MinHeapNode** array; 
}; 

struct MinHeapNode* newNode(char data, unsigned freq) 
{ 
    struct MinHeapNode* temp = (struct MinHeapNode*)malloc(sizeof(struct MinHeapNode)); 
    temp->left = temp->right = NULL; 
    temp->data = data; 
    temp->freq = freq; 
    return temp; 
} 

struct MinHeap* createMinHeap(unsigned capacity) 
{ 
    struct MinHeap* minHeap = (struct MinHeap*)malloc(sizeof(struct MinHeap)); 
    minHeap->size = 0; 
    minHeap->capacity = capacity; 
    minHeap->array = (struct MinHeapNode**)malloc(minHeap->capacity * sizeof(struct MinHeapNode*)); 
    return minHeap; 
} 

void swapMinHeapNode(struct MinHeapNode** a, struct MinHeapNode** b) 
{ 
    struct MinHeapNode* t = *a; 
    *a = *b; 
    *b = t; 
} 

void minHeapify(struct MinHeap* minHeap, int idx) 
{ 
    int smallest = idx; 
    int left = 2 * idx + 1; 
    int right = 2 * idx + 2; 

    if (left < minHeap->size && minHeap->array[left]->freq < minHeap->array[smallest]->freq) 
        smallest = left; 

    if (right < minHeap->size && minHeap->array[right]->freq < minHeap->array[smallest]->freq) 
        smallest = right; 

    if (smallest != idx) { 
        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]); 
        minHeapify(minHeap, smallest); 
    } 
} 

int isSizeOne(struct MinHeap* minHeap) 
{ 
    return (minHeap->size == 1); 
} 

struct MinHeapNode* extractMin(struct MinHeap* minHeap) 
{ 
    struct MinHeapNode* temp = minHeap->array[0]; 
    minHeap->array[0] = minHeap->array[minHeap->size - 1]; 
    --minHeap->size; 
    minHeapify(minHeap, 0); 
    return temp; 
} 

void insertMinHeap(struct MinHeap* minHeap, struct MinHeapNode* minHeapNode) 
{ 
    ++minHeap->size; 
    int i = minHeap->size - 1; 

    while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq) { 
        minHeap->array[i] = minHeap->array[(i - 1) / 2]; 
        i = (i - 1) / 2; 
    } 
    minHeap->array[i] = minHeapNode; 
} 

void buildMinHeap(struct MinHeap* minHeap) 
{ 
    int n = minHeap->size - 1; 
    for (int i = (n - 1) / 2; i >= 0; --i) 
        minHeapify(minHeap, i); 
} 

int isLeaf(struct MinHeapNode* root) 
{ 
    return !(root->left) && !(root->right); 
} 

struct MinHeap* createAndBuildMinHeap(char data[], int freq[], int size) 
{ 
    struct MinHeap* minHeap = createMinHeap(size); 
    for (int i = 0; i < size; ++i) 
        minHeap->array[i] = newNode(data[i], freq[i]); 
    minHeap->size = size; 
    buildMinHeap(minHeap); 
    return minHeap; 
} 

struct MinHeapNode* buildHuffmanTree(char data[], int freq[], int size) 
{ 
    struct MinHeapNode *left, *right, *top; 
    struct MinHeap* minHeap = createAndBuildMinHeap(data, freq, size); 

    while (!isSizeOne(minHeap)) { 
        left = extractMin(minHeap); 
        right = extractMin(minHeap); 
        top = newNode('$', left->freq + right->freq); 
        top->left = left; 
        top->right = right; 
        insertMinHeap(minHeap, top); 
    } 
    return extractMin(minHeap); 
} 

void storeCodes(struct MinHeapNode* root, int arr[], int top, char **codes)
{ 
    if (root->left) { 
        arr[top] = 0; 
        storeCodes(root->left, arr, top + 1, codes); 
    } 

    if (root->right) { 
        arr[top] = 1; 
        storeCodes(root->right, arr, top + 1, codes); 
    } 

    if (isLeaf(root)) { 
        codes[(unsigned char)root->data] = (char *)malloc(top + 1);
        for (int i = 0; i < top; ++i) 
            codes[(unsigned char)root->data][i] = arr[i] + '0';
        codes[(unsigned char)root->data][top] = '\0';
    } 
}

typedef struct {
    FILE *inputFile;
    FILE *outputFile;
    char **codes;
    long start;
    long end;
} ThreadData;

void *saveToFileThread(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    fseek(data->inputFile, data->start, SEEK_SET);

    char c;
    while (ftell(data->inputFile) < data->end && (c = fgetc(data->inputFile)) != EOF) {
        fputs(data->codes[(unsigned char)c], data->outputFile);
    }

    return NULL;
}

void saveToFile(char *filename, char **codes) {
    FILE *inputFile = fopen(filename, "r");
    if (!inputFile) {
        fprintf(stderr, "Could not open input file %s\n", filename);
        return;
    }

    FILE *outputFile = fopen("output.txt", "w");
    if (!outputFile) {
        fclose(inputFile);
        fprintf(stderr, "Could not open output file output.txt\n");
        return;
    }

    fseek(inputFile, 0, SEEK_END);
    long fileSize = ftell(inputFile);
    long chunkSize = fileSize / NUM_THREADS;
    long lastChunkSize = fileSize - chunkSize * (NUM_THREADS - 1);

    pthread_t threads[NUM_THREADS];
    ThreadData threadData[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        threadData[i].inputFile = inputFile;
        threadData[i].outputFile = outputFile;
        threadData[i].codes = codes;
        threadData[i].start = i * chunkSize;
        threadData[i].end = (i == NUM_THREADS - 1) ? (i * chunkSize + lastChunkSize) : ((i + 1) * chunkSize);

        if (pthread_create(&threads[i], NULL, saveToFileThread, &threadData[i]) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            fclose(inputFile);
            fclose(outputFile);
            return;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    fclose(inputFile);
    fclose(outputFile);
}

void HuffmanCodes(char data[], int freq[], int size, char *filename) 
{ 
    struct MinHeapNode* root = buildHuffmanTree(data, freq, size); 
    char *codes[NUM_CHARS] = {0}; 
    int arr[MAX_TREE_HT], top = 0; 
    storeCodes(root, arr, top, codes); 

    saveToFile(filename, codes);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stdout, "Uso: %s <N> <M> <B> <arquivo_texto>\n", argv[0]);
        return -1;
    }

    N = atoi(argv[1]);
    M = atoi(argv[2]);
    B = atoi(argv[3]);
    file = fopen(argv[4], "r");

    if (!file) {
        fprintf(stderr, "Erro ao abrir o arquivo.\n");
        return -1;
    }

    Buffer = (char **)malloc(M * sizeof(char *));
    if (Buffer == NULL) {
        fprintf(stdout, "Erro de alocação de memória.\n");
        return -2;
    }

    sem_init(&slotCheio, 0, 0);
    sem_init(&slotVazio, 0, M);
    sem_init(&mutexGeral, 0, 1);

    pthread_t prod_thread;
    pthread_t cons_threads[N];
    long long int *counts[N];

    if (pthread_create(&prod_thread, NULL, produtor, NULL) != 0) {
        fprintf(stderr, "Erro ao criar a thread produtora.\n");
        return -1;
    }

    for (int i = 0; i < N; i++) {
        if (pthread_create(&cons_threads[i], NULL, consumidor, NULL) != 0) {
            fprintf(stderr, "Erro ao criar a thread consumidora %d.\n", i);
            return -1;
        }
    }

    pthread_join(prod_thread, NULL);

    long long int finalCounts[NUM_CHARS] = {0};
    for (int i = 0; i < N; i++) {
        void *res;
        pthread_join(cons_threads[i], &res);
        counts[i] = (long long int *)res;
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < NUM_CHARS; j++) {
            finalCounts[j] += counts[i][j];
        }
        free(counts[i]);
    }

    int numDistinctChars = 0;
    for (int i = 0; i < NUM_CHARS; i++) {
        if (finalCounts[i] > 0) {
            numDistinctChars++;
        }
    }

    char data[numDistinctChars];
    int freq[numDistinctChars];
    int index = 0;

    for (int i = 0; i < NUM_CHARS; i++) {
        if (finalCounts[i] > 0) {
            data[index] = (char)i;
            freq[index] = finalCounts[i];
            index++;
        }
    }

    HuffmanCodes(data, freq, numDistinctChars, argv[4]);

    fclose(file);
    free(Buffer);

    return 0;
}