pid=$(./getPid.sh)

# Check if the previous command was successful
if [ $? -ne 0 ]; then
    echo "Failed to get sigHandler PID. Error message:"
    echo "$pid"
    exit 1
fi

# Check if the process exists
if ! kill -0 $pid 2>/dev/null; then
    echo "Process with PID $pid does not exist."
    exit 1
fi

# Array of common signal numbers and their names
declare -a signals=(
    "1:HUP" "2:INT" "3:QUIT" "4:ILL" "5:TRAP" "6:ABRT" "7:BUS" "8:FPE" "9:KILL" 
    "10:USR1" "11:SEGV" "12:USR2" "13:PIPE" "14:ALRM" "15:TERM" "16:STKFLT" 
    "17:CHLD" "18:CONT" "19:STOP" "20:TSTP" "21:TTIN" "22:TTOU" "23:URG" 
    "24:XCPU" "25:XFSZ" "26:VTALRM" "27:PROF" "28:WINCH" "29:IO" "30:PWR"
)

for sig in "${signals[@]}"; do
    num=${sig%%:*}
    name=${sig#*:}
    echo "Sending signal $num (SIG$name) to process $pid"
    if [ $num == 9 ]; then
        continue
    fi
    kill -$num $pid 2>/dev/null
    sleep 0.1  # Small delay to avoid overwhelming the process
    
    # Check if the process still exists
    if ! kill -0 $pid 2>/dev/null; then
        echo "Process $pid terminated after receiving signal $num (SIG$name)"
        exit 0
    fi
done

echo "Process $pid survived all signals"
