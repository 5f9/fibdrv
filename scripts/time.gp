reset
set title 'runtime O(n)'
set xlabel 'size'
set ylabel 'time(ns)'
set key left top
set term png enhanced size 1024,768 font 'Verdana,10'
set output 'logs/time.png'
plot [:370][:15000] \
'logs/user_doubling_256_clz_time.dat' title "user doubling 256 clz" with linespoints lc rgb 'green', \
'logs/user_sequence_256_time.dat' title "user sequence 256" with linespoints lc rgb 'orange', \
'logs/kernel_doubling_256_clz_time.dat' title "kernel doubling 256 clz" with linespoints lc rgb 'green', \
'logs/kernel_sequence_256_time.dat' title "kernel sequence 256" with linespoints lc rgb 'orange', \
'logs/kernel_to_user_time.dat' title "kernel to user" with linespoints lc rgb 'violet', \
'logs/user_to_kernel_time.dat' title "user to kernel" with linespoints, \
'logs/user_doubling_clz_time.dat' title "user doubling clz" with linespoints lc rgb 'green', \
'logs/user_doubling_time.dat' title "user doubling" with linespoints lc rgb 'brown', \
'logs/user_sequence_time.dat' title "user sequence" with linespoints lc rgb 'orange', \
'logs/kernel_doubling_clz_time.dat' title "kernel doubling clz" with linespoints lc rgb 'green', \
'logs/kernel_doubling_time.dat' title "kernel doubling" with linespoints lc rgb 'brown', \
'logs/kernel_sequence_time.dat' title "kernel sequence" with linespoints lc rgb 'orange'