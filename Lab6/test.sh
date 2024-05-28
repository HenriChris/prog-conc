#!/bin/bash

numOfElementsArray=(10 20 50 100 500 1000 10000)
numOfThreadsArray=(1 2 3 4 5 6 7 8)
bufferSizeArray=(1 2 5 10 20 50 100 500 1000)

gcc generateArray.c -o generateArray -Wall -ansi -pedantic -std=c2x -lm
gcc isPrime.c -o isPrime -Wall -ansi -pedantic -std=c2x -lm

success=true

for i in "${numOfElementsArray[@]}"; do
    ./generateArray $i ./array
    for j in "${numOfThreadsArray[@]}"; do
        for k in "${bufferSizeArray[@]}"; do
            ./isPrime $j $k ./array
            result=$?

            if [ $result -eq 1 ]; then
                echo "Erro : Número de elementos $i, Número de threads $j e Tamanho de buffer $k"
                success=false
            fi
        done
    done
done

if [ "$success" = true ]; then
    echo "Todos os testes foram um sucesso!"
fi