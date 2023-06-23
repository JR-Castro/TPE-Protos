#!/bin/bash

for ((i=1; i<=5; i++)) # change to 500
do
  nc -C -v localhost 1110 &
#  printf "USER user%s\nPASS password\n", "$i" | nc -C -v localhost 1110 & #to log a user
done

##killall nc -9