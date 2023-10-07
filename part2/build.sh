#!/usr/bin/bash -ue

gcc -Wall repeat_fread.c -o repeat_fread

# gcc -g -Wall haversine_sum.c -o haversine_sum
gcc -g -Wall haversine_sum.c -DPROFILE -o haversine_sum_p

#echo No profiler
#./haversine_sum
#echo
./haversine_sum_p
