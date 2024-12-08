#!/usr/bin/bash -ue

gcc -g -O2 -Wall store_latency.c -o store_latency

#nasm -f win64 cache_tester.asm -o cache_tester.obj
#gcc -g -Og -Wall cache_tester.c cache_tester.obj -o cache_tester

#nasm -f win64 rw_port_tester.asm -o rw_port_tester.obj
#gcc -g -Og -Wall rw_port_tester.c rw_port_tester.obj -o rw_port_tester

# nasm -f win64 write_bytes.asm -o write_bytes.obj
# gcc -g -Og -Wall repeat_write_bytes.c write_bytes.obj -o repeat_write_bytes

# gcc -g -Wall haversine_sum.c -o haversine_sum
# gcc -g -Wall haversine_sum.c -DPROFILE -o haversine_sum_p

#echo No profiler
#./haversine_sum
#echo
#./haversine_sum_p
