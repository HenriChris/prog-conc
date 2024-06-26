#!/bin/bash

numOfConsumerThreadsArray=(1 2 3 4 5 6 7 8)
blockSizeArray=(1000 5000 10000)
bufferSizeArray=(1 2 5 10 20)

gcc huffman4.c -o huffman4 -Wall -ansi -pedantic -std=c2x
touch result.txt

for i in "${numOfConsumerThreadsArray[@]}"; do
    for j in "${bufferSizeArray[@]}"; do
        for k in "${blockSizeArray[@]}"; do
        echo "Número de Threads $i, Tamanho do buffer $j e Tamanho do bloco $k :" >> result.txt
            for l in {1..5}; do
                (time ./huffman4 $i $j $k bible.txt) 2>> result.txt
            done
        done
    done
done
