#!/bin/bash

# Read input from standard input
IFS= read -r first_line
while IFS= read -r line; do
  echo "$line"
done

