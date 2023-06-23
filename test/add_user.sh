#!/bin/bash

## ABSTRACT
# This script is used to add users to the server and then login with them.
# It is used to test the server's incoming connections with multiple users.

## STEPS TO RUN THIS TEST
# 1. Start the pop3 server with the following command
# 2. Run this script with the following command: ./add_user.sh (default cant of users is 500)
# 3. To remove the folder created during the test, run the script ./remove_test_folders.sh

# Cant of users to be created
users=500
# Export token to be able to use the manager client
export TOKEN=12345678
# For-loop to add the users credentials to the server and exit
for ((i = 1; i <= $users; i++)); do
  ~/workspace/protos/TPE-Protos/cmake-build-debug/src/managerclient 127.0.0.1 9090 <<EOF
add-user user$i:password
exit
EOF
done

# For-loop to connect to the server and login with the users credentials
for ((i=1; i<=$users; i++))
do
  # Creat user's directory otherwise the login will fail
  mkdir ~/workspace/protos/.mails/user$i;
  # Run in separate threads to simulate multiple users
  printf "USER user%s\nPASS password\n" $i | nc -C -v localhost 1110 &
done