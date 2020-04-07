#!/bin/bash
for i in {1..300}
do
    echo "Welcome $i times"
    ./trudpcat_ev_wq 127.0.0.1 10110 &
    sleep 0.01
done 