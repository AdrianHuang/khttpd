#!/bin/bash

# usage:
#   ./fib_valid_profiling.sh <start_number> <end_number> <step>
#
# Borrow the code from: https://gist.github.com/a60814billy/4fd114c860b715c86c8dffc8997f1c62?fbclid=IwAR1T3vZ7AjkY3AlZ_uJ5YlSg1zlVhGC-dxvLWwGCfn_X01BvQYBITooqbps

DATA_FILE=fib-orig.log

DFL_FIB_NUM_START=0
DFL_FIB_NUM_END=50000
DFL_FIB_NUM_STEP=1000

out_path=`dirname $0`

fib_compare_answer() {
    local expect="$(curl -s http://www.protocol5.com/Fibonacci/$1.htm | grep -oP 'Decimal\s+Base\s+10.*<div>\K\d+')"

    if [[ -z "$expect" ]]; then
        echo "fail to fetch fibonacci number from http://www.protocol5.com/Fibonacci/$1.htm"
        return 1;
    fi

    if [[ "$expect" != "$2" ]]; then
        echo "fail, fib($1) = $expect, not $2"
        return 2;
    fi
    echo "ok, fib($1) = $expect"
    return 0;
}

if [ $# -ge 3 ]; then
    fib_num_start=$1
    fib_num_end=$2
    fib_num_step=$3
else
    fib_num_start=$DFL_FIB_NUM_START
    fib_num_end=$DFL_FIB_NUM_END
    fib_num_step=$DFL_FIB_NUM_STEP
fi

# Empty file
> $out_path/$DATA_FILE

for i in $(seq $fib_num_start $fib_num_step $fib_num_end)
do
    IFS='.'

    read -a start_time <<< $(date +"%s.%6N")
    fib_answer_from_khttpd=$(curl -s http://localhost:8081/fib/$i)
    read -a end_time <<< $(date +"%s.%6N")

    # Extract second field and milisecond field
    sec=`echo "${end_time[0]} - ${start_time[0]}" | bc`
    ms=`echo "${end_time[1]} - ${start_time[1]}" | bc`
   
    # Time spent in milisecond unit
    total_ms=`echo "$sec * 1000000 + $ms" | bc`

    IFS=''

    echo "$i, $total_ms" >> $out_path/$DATA_FILE

    fib_compare_answer $i $fib_answer_from_khttpd
    if [ $? -ne 0 ]; then
        # The fib number is incorrect. Exit.
        exit 1
    fi
done

cd $out_path
gnuplot data.gp

exit 0
