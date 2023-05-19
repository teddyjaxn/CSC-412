#!/bin/bash

if [ -n "$3" ]; then
    echo "usage: ./script01.sh <path to data file>  <output file path> "
elif [ -z "$2" ]; then
    echo "usage: ./script01.sh <path to data file>  <output file path> "
else


    if [ ${1:0:2} != "./" ]; then
        
        if [ ${1:0:1} == "/" ]; then
            Dir=${1/#"/"/"./"}
        else
            Dir="./${1}"
        fi
    fi
    my_array=($(find "$Dir" -maxdepth 1 -name "*xt" -type f))
    num=-1
    if [ ! -d "./temp" ] ; then
        mkdir ./temp
    fi
    if [ ${#my_array[@]} == "0" ]; then
        echo "No file(s) found in $Dir."
    else
        for i in ${my_array[@]}
        do
            foo=${i#$Dir}
            proc=$(grep -o '^[0-9]*' "$Dir$foo")
            if [ $proc -gt $num ]; then
                num=$proc
            fi
        done
    fi
    
    $2Prog4 $((++num)) $Dir

fi