set title "Intel(R) Core(TM) i5 CPU M 520 @ 2.40GHz - 4 cores (HT enabled)"
set xlabel "Fibonacci number: Fn"
set ylabel "Time Cost (ms)"
set term png enhanced font 'Verdana,10'
set output "fib.png"
set key left

plot \
"fib-orig.log" using 1:2 with linespoints linewidth 2 title "fib-orig",
