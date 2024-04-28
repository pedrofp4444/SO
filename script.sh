#!/bin/bash

# Define the command to execute
COMMAND="./bin/client execute -u"

# Define the file to use as an argument
FILE="README.md"

# Define an array of different values of x
X_VALUES=(100 200 300 400 500 600 700)

# Define the number of clients to run simultaneously
NUM_CLIENTS=${#X_VALUES[@]}

# Run clients simultaneously
for ((i=0; i<NUM_CLIENTS; i++)); do
    X=${X_VALUES[$i]}
    $COMMAND $X "cat $FILE" &  # Execute the command in the background
done

# Wait for all background processes to finish
wait
