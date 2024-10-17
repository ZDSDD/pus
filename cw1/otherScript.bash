#!/bin/bash

# Capture the output of the get_sighandler_pid.sh script
sighandler_pid=$(./getPid.sh)

# Check if the previous command was successful
if [ $? -ne 0 ]; then
    echo "Failed to get sigHandler PID. Error message:"
    echo "$sighandler_pid"
    exit 1
fi

# Check if the output is a valid PID (a number)
if ! [[ "$sighandler_pid" =~ ^[0-9]+$ ]]; then
    echo "Invalid PID received: $sighandler_pid"
    exit 1
fi

echo "sigHandler PID: $sighandler_pid"

# Now you can use $sighandler_pid in your script
# For example:

# Send a signal to the process
echo "Sending SIGUSR1 to the process..."
kill -SIGUSR1 $sighandler_pid

# Get more information about the process
echo "Process information:"
ps -p $sighandler_pid -o pid,ppid,cmd,start,time

# Add more operations here as needed

echo "Script completed successfully."
