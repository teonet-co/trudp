#!/bin/bash
echo "Usage: ./trudpcat.sh <number_of_clients>"
for i in `seq 1 $1`;
do
  exec dist/Debug/GNU-Linux/trudpcat -Q 127.0.0.1 8000 $2 &
done
