#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <number_of_clients>"
    exit 1
fi

NUM_CLIENTS=$1

for ((i=0; i<NUM_CLIENTS; i++))
do
    START=$((RANDOM % 1000 + 1))
    END=$((START + RANDOM % 1000000))
    ./client $START $END &
done

wait
