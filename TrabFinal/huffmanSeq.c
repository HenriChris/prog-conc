#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_CHARS 256

// Nó da árvore binária de Huffman
struct MinHeapNode
{
    char data; // Caracter correspondente
    int freq; // Frequência do caracter correspondente no arquivo de entrada
    struct MinHeapNode *left, *right; // Filhos do nó
};

// Coleção de nós
struct MinHeap
{
    int size;
    int capacity;
    struct MinHeapNode** array;
};

// Inicializa um novo nó
struct MinHeapNode* initNode(char data, int freq);

// Inicializa uma nova Min Heap
struct MinHeap* initMinHeap(int capacity);

// Troca dois nós da Min Heap
void swapMinHeapNode(struct MinHeapNode** a, struct MinHeapNode** b);

// Ordena o vetor (ou a parte do vetor correspondente a uma subárvore com raiz idx)
// de modo que tenha as propriedades de uma minheap
void minHeapify(struct MinHeap* minHeap, int idx);

// Retorna o menor valor da heap e remove o nó correspondente da árvore
struct MinHeapNode* extractMin(struct MinHeap* minHeap);

// Insere um novo nó na heap
void insertMinHeap(struct MinHeap* minHeap, struct MinHeapNode* minHeapNode);

// Ordena o vetor de modo que tenha as propriedades de uma minHeap
void buildMinHeap(struct MinHeap* minHeap);

// Cria uma minHeap com nós possuindo chave data e valor freq
struct MinHeap* createAndBuildMinHeap(char data[], long long int freq[], int size);

// Monta a árvore de huffman a partir dos dados obtidos
struct MinHeapNode* buildHuffmanTree(char data[], long long int freq[], int size);

// Função para contar a frequência de cada caracter presente no arquivo de entrada
long long int countCharacters(const char *filename, long long int *freq, char* chars);

// Função para montar o dicionário de codificação
void storeCodes(struct MinHeapNode* root, char** huffmanCodes, char* code, int top);

// Função que retorna o dicionário de codificação pronto
char** dictFromHuffmanTree(struct MinHeapNode* root);

// Função que codifica a entrada do código e salva o resultado em um output
void encodeString(const char *inputFileName, const char *outputFileName,  char** huffmanCodes);

// Função para liberar memória da árvore de Huffman
void freeHuffmanTree(struct MinHeapNode* root);

// Função para liberar memória do dicionário de Huffman
void freeHuffmanDict(char** huffmanCodes);

int main(int argc, char *argv[]) 
{
    long long int* freq;
    char* chars;
    long long int nonZeros;
    clock_t begin;
    clock_t end;

    if (argc != 3)
    {
        fprintf(stdout, "Uso correto: %s <path do arquivo de entrada> <path do arquivo de saída>\n", argv[0]);
        return -1;
    }

    freq = (long long int*)calloc(NUM_CHARS, sizeof(long long int));
    if (freq == NULL)
    {
        fprintf(stdout, "Erro de alocação de memória.\n");
        return -2;
    }

    chars = (char*)malloc(NUM_CHARS * sizeof(char));
    if (chars == NULL)
    {
        fprintf(stdout, "Erro de alocação de memória.\n");
        free(freq);
        return -2;
    }

    begin = clock();
    nonZeros = countCharacters(argv[1], freq, chars);
    end = clock();
    printf("Tempo de execução para contagem de caracteres : %f\n", (double)(end - begin) / CLOCKS_PER_SEC);

    begin = clock();
    struct MinHeapNode* root = buildHuffmanTree(chars, freq, nonZeros);
    char** huffmanDict = dictFromHuffmanTree(root);
    end = clock();
    printf("Tempo de execução para montar a árvore de huffman: %f\n", (double)(end - begin) / CLOCKS_PER_SEC);

    begin = clock();
    encodeString(argv[1], argv[2], huffmanDict);
    end = clock();
    printf("Tempo de execução para codificar e salvar a saida: %f\n", (double)(end - begin) / CLOCKS_PER_SEC);

    freeHuffmanDict(huffmanDict);
    freeHuffmanTree(root);
    free(chars);
    free(freq);

    return 0;
}

struct MinHeapNode* initNode(char data, int freq)
{
    struct MinHeapNode* temp = (struct MinHeapNode*)malloc(sizeof(struct MinHeapNode));
    if (temp == NULL)
    {
        fprintf(stdout, "Erro de alocação de memória.\n");
        exit(-2);
    }
    temp->left = temp->right = NULL;
    temp->data = data;
    temp->freq = freq;
    return temp;
}

struct MinHeap* initMinHeap(int capacity)
{
    struct MinHeap* minHeap = (struct MinHeap*)malloc(sizeof(struct MinHeap));
    if (minHeap == NULL)
    {
        fprintf(stdout, "Erro de alocação de memória.\n");
        exit(-2);
    }
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array = (struct MinHeapNode**)malloc(sizeof(struct MinHeapNode*) * capacity);
    if (minHeap->array == NULL)
    {
        fprintf(stdout, "Erro de alocação de memória.\n");
        free(minHeap);
        exit(-2);
    }
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
    // Indices dos dois filhos de idx no vetor minHeap
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->array[left]->freq < minHeap->array[smallest]->freq)
        smallest = left;

    if (right < minHeap->size && minHeap->array[right]->freq < minHeap->array[smallest]->freq)
        smallest = right; 

    if (smallest != idx)
   
    {
        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

struct MinHeapNode* extractMin(struct MinHeap* minHeap)
{
    if (minHeap->size == 0)
    {
        fprintf(stdout, "MinHeap is empty.\n");
        return NULL;
    }
    struct MinHeapNode* temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    minHeap->size--;
    minHeapify(minHeap, 0);
    return temp;
}

void insertMinHeap(struct MinHeap* minHeap, struct MinHeapNode* minHeapNode)
{
    int i = minHeap->size;
    minHeap->size++;

    // (i - 1)/2 é o índice do nó pai de i.
    while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq)
    {
        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    minHeap->array[i] = minHeapNode;
}

void buildMinHeap(struct MinHeap* minHeap)
{
    int n = minHeap->size - 1;
    for (int i = (n - 1) / 2; i >= 0; i--)
        minHeapify(minHeap, i);
}

struct MinHeap* createAndBuildMinHeap(char data[], long long int freq[], int size)
{
    struct MinHeap* minHeap = initMinHeap(size);
    for (int i = 0; i < size; ++i)
        minHeap->array[i] = initNode(data[i], freq[i]);
    minHeap->size = size;
    buildMinHeap(minHeap);
    return minHeap;
}

struct MinHeapNode* buildHuffmanTree(char data[], long long int freq[], int size)
{
    struct MinHeapNode *left, *right, *top;
    struct MinHeap* minHeap = createAndBuildMinHeap(data, freq, size);

    while (minHeap->size > 1)
    {
        left = extractMin(minHeap);
        right = extractMin(minHeap);

        // Cria um novo nó com a soma das frequências dos menores nós
        // Terá valor '\b', que não será usado        
        top = initNode('\0', left->freq + right->freq);

        top->left = left;
        top->right = right;
        
        insertMinHeap(minHeap, top);
    }

    struct MinHeapNode* root = extractMin(minHeap);
    free(minHeap->array);
    free(minHeap);
    return root;
}

long long int countCharacters(const char *filename, long long int *freq, char* chars)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        fprintf(stdout, "Erro ao abrir arquivo de texto.\n");
        exit(-3);
    }

    // Para cada caracter do arquivo, incrementa um no endereço do vetor contador 
    // correspondente ao valor ascii do caracter.
    // Ex : caracter 'a' tem valor 91 -> counts[91] += 1;
    int c;
    while ((c = fgetc(file)) != EOF)
        freq[(unsigned char)c]++;

    fclose(file);

    long long int nonZeros = 0;
    for (int i = 0; i < NUM_CHARS; i++)
    {
        if (freq[i] > 0)
    {
            chars[nonZeros] = (char)i;
            freq[nonZeros] = freq[i];
            nonZeros++;
        }
    }
    return nonZeros;
}

void storeCodes(struct MinHeapNode* root, char** huffmanCodes, char* code, int top)
{   
    if (root->left)
    {
        code[top] = '0';
        storeCodes(root->left, huffmanCodes, code, top + 1);
    }

    if (root->right)
    {
        code[top] = '1';
        storeCodes(root->right, huffmanCodes, code, top + 1);
    }

    if (!root->left && !root->right)
    {
        code[top] = '\0';
        huffmanCodes[(unsigned char)root->data] = strdup(code);
    }
}

char** dictFromHuffmanTree(struct MinHeapNode* root)
{
    char** huffmanCodes = (char**)malloc(sizeof(char*) * NUM_CHARS);
    if (huffmanCodes == NULL)
    {
        fprintf(stdout, "Erro de alocação de memória.\n");
        exit(-2);
    }

    for (int i = 0; i < NUM_CHARS; i++)
    {
        huffmanCodes[i] = NULL;
    }

    char code[NUM_CHARS];
    storeCodes(root, huffmanCodes, code, 0);

    return huffmanCodes;
}

void encodeString(const char *inputFileName, const char *outputFileName, char** huffmanCodes)
{
    FILE *inputFile = fopen(inputFileName, "r");
    if (inputFile == NULL)
    {
        fprintf(stdout, "Erro ao abrir arquivo de texto.\n");
        exit(-3);
    }

    FILE *outputFile = fopen(outputFileName, "w");
    if (outputFile == NULL)
    {
        fprintf(stdout, "Erro ao abrir arquivo de texto.\n");
        fclose(inputFile);
        exit(-3);
    }

    int c;
    while ((c = fgetc(inputFile)) != EOF)
    {
        if (huffmanCodes[(unsigned char)c] == NULL)
    {
            fprintf(stderr, "Caracter '%c' não está presente na árvore de huffman.\n", c);
            fclose(inputFile);
            fclose(outputFile);
            exit(-4);
        }
        fputs(huffmanCodes[(unsigned char)c], outputFile);
    }

    fclose(inputFile);
    fclose(outputFile);
}

void freeHuffmanTree(struct MinHeapNode* root)
{
    if (root == NULL) return;
    freeHuffmanTree(root->left);
    freeHuffmanTree(root->right);
    free(root);
}

void freeHuffmanDict(char** huffmanCodes)
{
    for (int i = 0; i < NUM_CHARS; i++)
    {
        if (huffmanCodes[i] != NULL)
    {
            free(huffmanCodes[i]);
        }
    }
    free(huffmanCodes);
}
