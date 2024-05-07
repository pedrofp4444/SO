#!/bin/bash

# Check if at least two arguments were provided
if [ $# -lt 3 ]; then
    echo "Usage: $0 <result_filename> <argument> [num_tasks]"
    exit 1
fi

# Define the second argument as the replacement for 3 in commands
ARGUMENT=$2

# Define the command to execute
COMMAND="./bin/client execute -u"

# Define the file to use as an argument
FILE="README.md"

# Define an array of different x values
X_VALUES=(100 200 300 400 500 600 700 800 900 1000)

# Define an array of commands to execute
COMMANDS=(
    "./test/test-programs/hello $ARGUMENT"
    "./test/test-programs/void $ARGUMENT"
)

# Define the number of tasks to check
# If an argument for the number of tasks is not provided, use a default of 7
NUM_TASKS=$3

# Define a function to generate a random X value
generate_random_X() {
    echo $(( (RANDOM % 901) + 100 ))
}

# Execute clients simultaneously
for ((i = 0; i < NUM_TASKS; i++)); do
    X=$(generate_random_X)
    COMMAND_TO_EXECUTE="${COMMANDS[$i % ${#COMMANDS[@]}]}"  # Cycle through commands if there are more clients than commands
    $COMMAND $X "$COMMAND_TO_EXECUTE" &  # Execute the command in the background
done

# Wait for all background processes to finish
wait

# Define the log file path
LOG_FILE="log"

# Check if the log file exists and has a size greater than zero
while [ ! -s "$LOG_FILE" ]; do
    echo "Creating results ..."
    sleep 0.5  # Wait half a second before checking again
done

# Check until the log file contains at least `NUM_TASKS` lines
while :; do
    line_count=$(wc -l < "$LOG_FILE")  # Count the number of lines in the log file
    if [ "$line_count" -ge "$NUM_TASKS" ]; then
        break
    fi
    sleep 0.5  # Wait half a second before checking again
done

# Define the directory to store the result file
RESULTS_DIR="./test/results"

# Check if the results directory exists and create it if not
if [ ! -d "$RESULTS_DIR" ]; then
    mkdir -p "$RESULTS_DIR"
fi

# Define the path for the result file based on the provided argument
RESULT_FILE="$RESULTS_DIR/$1"

# Initialize the sum of durations
total_duration=0

# Read the log file and calculate the sum of durations
task_count=0  # Variable to count the number of tasks
while read -r line; do
    # Extract the duration from the line
    duration=$(echo "$line" | grep -oP "Duration: \K[\d\.]+")
    
    # Add the duration to the total variable
    total_duration=$(echo "$total_duration + $duration" | bc)

    # Increment the task counter
    ((task_count++))
done < "$LOG_FILE"

# Calculate the average duration
if [ $task_count -gt 0 ]; then
    average_duration=$(echo "scale=2; $total_duration / $task_count" | bc)
else
    average_duration=0
fi

# Format the results correctly with two decimal places
total_duration_formatted=$(printf "%.6f" "$total_duration")
average_duration_formatted=$(printf "%.6f" "$average_duration")

# Open the result file for writing
{
    echo "Total duration: $total_duration_formatted seconds"
    echo "Average duration per task: $average_duration_formatted seconds"
} > "$RESULT_FILE"

echo "Results saved to $RESULT_FILE"
