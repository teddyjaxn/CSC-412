#!/bin/bash

if [ -n "$4" ]; then
    echo "usage: ./script02.sh <path to drop folder> <path to data folder>  <path to output file> "
elif [ -z "$3" ]; then
    echo "usage: ./script02.sh <path to drop folder> <path to data folder>  <path to output file>  "
else

    if [ ${1:0:2} != "./" ]; then
        
        if [ ${1:0:1} == "/" ]; then
            Dir1=${1/#"/"/"./"}
        else
            Dir1="./${1}"
        fi
    else
        Dir1=$1
    fi
    if [ ${2:0:2} != "./" ]; then
        
        if [ ${2:0:1} == "/" ]; then
            Dir2=${2/#"/"/"./"}
        else
            Dir2="./${2}"
        fi
    else
        Dir2=$2
    fi
    if [ ${3:0:2} != "./" ]; then
        
        if [ ${3:0:1} == "/" ]; then
            Dir3=${3/#"/"/"./"}
        else
            Dir3="./${3}"
        fi
    else
        Dir3=$3
    fi
    echo " all file paths good"
    echo $Dir1 $Dir2 $Dir3
    if [ "$Dir1" != "$Dir2" ]; then
            echo " first check good" 
        if [ "$Dir1" != "$Dir3" ]; then
            echo " second check good" 
            if [ "$Dir2" != "$Dir3" ]; then 
            echo " third check good" 
                if [ ! -d "$Dir1" ]; then
                    mkdir "$Dir1"
                fi
                if [ ! -d "$Dir2" ]; then
                    mkdir "$Dir2"
                fi
                if [ ! -d "$Dir3" ]; then
                    mkdir "$Dir3"
                fi
                if [ ! -d "./Completed" ] ; then
                    mkdir "./Completed"
                fi

                inotifywait -m "$Dir1" -e moved_to |
                while read path action file; do
                echo "the file is named '$file' "
                if [[ $file == *.tga ]]; then
                    echo "Image detected"
                    impa="${Dir1}/${file}"
                    imde="${Dir2}/${file}"
                    mv $impa $imde
                elif [[ $file == *.job ]]; then
                #echo "./a.out $file $Dir2 $Dir3"
                
                jopa="${Dir1}/${file}"
                ./Prog5Version1.out $jopa $Dir2 $Dir3
                jode="./Completed/${file}"
                mv $jopa $jode

                fi
                done

            fi
        fi
    
    fi

    

fi