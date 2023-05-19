#!/bin/bash

if [ -n "$3" ]; then
    echo "usage: ./script02.sh <path> <extension>"
elif [ -z "$2" ]; then
    echo "usage: ./script02.sh <path> <extension>"
else
    my_array=($(find "$1" -maxdepth 1 -name "*.$2" -type f))
    echo "Looking for files with extension .$2 in folder"
    echo "     $1:"

    if [ ${#my_array[@]} == "0" ]; then
        echo "No file found."
    elif [ ${#my_array[@]} == "1" ]; then
        echo "1 file found:"
        for i in ${my_array[@]}
        do
            foo=${i#$1}
            echo "     "${foo#*/}
        done
    else
        echo "${#my_array[@]} files found:"
        for i in ${my_array[@]}
        do
            foo=${i#$1}
            echo "     "${foo#*/}
        done
    fi
fi