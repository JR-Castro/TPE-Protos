#!/bin/bash

# Cant of users that were created with add_user.sh
users=500
# Remove folders created for this test
for ((i=1; i<=$users; i++));
do
  rm -rf ~/workspace/protos/.mails/user$i;
done

printf "done\n"