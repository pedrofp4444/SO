#!/bin/bash

# Define o comando a ser executado
COMMAND="./bin/client execute -u"

# Define o arquivo a ser usado como argumento
FILE="README.md"

# Define os arquivos a serem usados como argumento
FILES=("README.md" "Makefile")

# Define os comandos a serem executados em um loop
COMMANDS=(
    "cat $FILE"
    "ls -l"
    "ps aux"
    "sleep 3 && echo 'Este é um comando atrasado'"
)

# Define os comandos para pipelines
PIPELINE_COMMANDS=(
    "grep 'bash' /etc/passwd | cut -d: -f1"
    "ls -l | grep '^d'"
)

# Define o número de vezes que cada comando deve ser executado
NUM_ITERATIONS=3

# Define os valores de tempo (X_VALUES)
X_VALUES=(100 200 300 400 500 600 700)

# Define o número de clientes a serem executados simultaneamente
NUM_CLIENTS=${#X_VALUES[@]}

# Executa os comandos em um loop
for ((i=0; i<NUM_ITERATIONS; i++)); do
    # Itera sobre os valores de X_VALUES
    for X in "${X_VALUES[@]}"; do
        # Executa os comandos com o valor atual de X
        for FILE_TO_USE in "${FILES[@]}"; do
            $COMMAND $X "ls $FILE_TO_USE" & 
            sleep 0.15 # Executa o comando com cada arquivo em segundo plano
        done
        for COMMAND_TO_EXECUTE in "${COMMANDS[@]}"; do
            $COMMAND $X "$COMMAND_TO_EXECUTE" &  # Executa o comando em segundo plano
            sleep 0.15
        done
        for PIPELINE_COMMAND_TO_EXECUTE in "${PIPELINE_COMMANDS[@]}"; do
            eval "$PIPELINE_COMMAND_TO_EXECUTE" &  # Executa o comando em segundo plano
            sleep 0.15
        done
        sleep 1  # Aguarda 1 segundo entre as execuções
    done
done

# Aguarda todas as execuções em segundo plano terminarem
wait

