#!/bin/bash

for ((i=1; i<=5; i++)) # change to 500
do
  gnome-terminal -- bash -c "cd /home/magui/Desktop/TPE-Protos/cmake-build-debug/src &&
                                 ./pop3server -d /home/magui/Desktop/TPE-Protos/test/user$i -u user$i:password; exec bash"

    sleep 1

  gnome-terminal -- bash -c "printf 'USER user%s\nPASS password\n' $i | nc -C -v localhost 1110; exec bash"
done

