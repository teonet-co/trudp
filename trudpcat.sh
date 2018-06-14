#!/bin/bash
echo "Usage: ./trudpcat.sh <number_of_clients>"
echo "Use killall lt-trudpcat_ev to stop all runnig examples"
for i in `seq 1 $1`;
do
  exec examples/trudpcat_ev 127.0.0.1 8000 $2 &
done

