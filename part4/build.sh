#!/usr/bin/env bash

g++ -Og -Wall -Wno-unknown-pragmas listing_0173_reference_haversine_main.cpp -o haversine

g++ -O2 -S listing_0173_reference_haversine_main.cpp -o haversine.s
