#!/bin/bash

# A script to unset the CPU frequency.

# Before use, modify necessary parameters
# to meet your machine's configuration.
for i in $(seq 0 3);
do
    sudo cpufreq-set -d 800000 -u 3000000 -c $i -r
done

sleep 5

for i in $(seq 0 3);
do
    sudo cpufreq-set -g powersave -c $i -r
done
# Type 'cpufreq-info' to verify
# the changes made.
