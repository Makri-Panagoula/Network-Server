#!/bin/bash 
#check that poll-log exists and we have the right permissions
if [ "$#" -eq 0 ]
    then echo "Give an output file!"
    exit 1
fi
if [ ! -e  $1 ]
    then echo "inputFile doesn't exist!"
    exit 1
fi    

if [ ! -r $1 ]  
    then echo "We don't have the needed permissions for output file!"
    exit 1
fi