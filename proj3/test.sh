#!/bin/bash

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <number_of_clients> <max_start> <max_end>"
    exit 1
fi

NUM_CLIENTS=$1
MAX_START=$2
MAX_END=$3

if [ $MAX_START -ge $MAX_END ]; then
    echo "Error: max_start must be less than max_end"
    exit 1
fi

for ((i=0; i<NUM_CLIENTS; i++))
do
    # Using bc for better handling of large numbers
    START=$(echo "1 + $(od -An -N8 -tu8 /dev/urandom) % $MAX_START" | bc)
    END=$(echo "$START + $(od -An -N8 -tu8 /dev/urandom) % ($MAX_END - $START)" | bc)
    ./client $START $END &
done

wait