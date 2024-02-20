#!/usr/bin/bash -ue

nasm -f win64 rw_port_tester.asm -o rw_port_tester.obj
gcc -g -Og -Wall rw_port_tester.c rw_port_tester.obj -o rw_port_tester

# nasm -f win64 write_bytes.asm -o write_bytes.obj
# gcc -g -Og -Wall repeat_write_bytes.c write_bytes.obj -o repeat_write_bytes

# gcc -g -Wall haversine_sum.c -o haversine_sum
# gcc -g -Wall haversine_sum.c -DPROFILE -o haversine_sum_p

#echo No profiler
#./haversine_sum
#echo
#./haversine_sum_p
