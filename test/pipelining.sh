#!/bin/bash

cd /home/magui/Desktop/TPE-Protos/cmake-build-debug/src
./pop3server -d /home/magui/Desktop/TPE-Protos/.mails -u magflores:password

printf "USER magflores\nPASS password\nCAPA\nLIST\n" | nc -C -v localhost 1110
