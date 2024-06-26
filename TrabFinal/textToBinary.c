#include <stdio.h>
#include <stdlib.h>

void convertTextToBinary(const char *inputFileName, const char *outputFileName) {
    FILE *inputFile = fopen(inputFileName, "r");
    FILE *outputFile = fopen(outputFileName, "wb");

    if (inputFile == NULL) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    if (outputFile == NULL) {
        perror("Error opening output file");
        fclose(inputFile);
        exit(EXIT_FAILURE);
    }

    int bit;
    unsigned char byte = 0;
    int count = 0;

    while ((bit = fgetc(inputFile)) != EOF) {
        if (bit == '0' || bit == '1') {
            byte = (byte << 1) | (bit - '0');
            count++;
            if (count == 8) {
                fwrite(&byte, sizeof(byte), 1, outputFile);
                byte = 0;
                count = 0;
            }
        }
    }

    // If there are remaining bits that do not make up a full byte, pad with zeros
    if (count > 0) {
        byte <<= (8 - count);  // Shift remaining bits to the left
        fwrite(&byte, sizeof(byte), 1, outputFile);
    }

    fclose(inputFile);
    fclose(outputFile);
}

int main() {
    const char *inputFileName = "encoded_bible.txt";
    const char *outputFileName = "output.bin";

    convertTextToBinary(inputFileName, outputFileName);

    printf("Conversion completed.\n");

    return 0;
}
