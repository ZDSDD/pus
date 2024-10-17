#!/bin/bash

pids=$(pgrep -x sigHandler)
pid_count=$(echo "$pids" | wc -l)

if [ -z "$pids" ]; then
    echo "No sigHandler process found." >&2
    return 1
elif [ "$pid_count" -eq 1 ]; then
    echo $(( $pids ))
else
    echo "Multiple sigHandler processes found. PIDs: $pids" >&2
    echo "Please specify which PID you want to use." >&2
    return 2
fi
