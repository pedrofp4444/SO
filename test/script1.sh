#!/bin/bash

# Define the command to execute
COMMAND="./bin/client execute -u"

# Define the file to use as an argument
FILE="README.md"

# Define an array of different values of x
X_VALUES=(100 200 300 400 500 600 700)

# Define an array of commands to execute
COMMANDS=(
    "cat $FILE"
    "ls -l"
    "ps aux"
    "sleep 3 && echo 'This is a delayed command'"
    "grep -v ˆ# /etc/passwd | cut -f7 -d: | uniq | wc -l"
    "ps aux | grep -v ˆUSER | wc -l"
    "ls -l /etc | grep ^d | wc -l"
)

# Define the number of clients to run simultaneously
NUM_CLIENTS=${#X_VALUES[@]}

# Run clients simultaneously
for ((i=0; i<NUM_CLIENTS; i++)); do
    X=${X_VALUES[$i]}
    COMMAND_TO_EXECUTE="${COMMANDS[$i % ${#COMMANDS[@]}]}"  # Cycling through commands if more clients than commands
    $COMMAND $X "$COMMAND_TO_EXECUTE" &  # Execute the command in the background
done

# Wait for all background processes to finish
wait
