#!/bin/bash 

#check validity of command line arguments
if [ ! -e $1]
    then echo "file doesn't exist"
    exit 1
fi    
if [ $2 -le 0 ]
    then echo "numLines should be positive"
    exit 1
fi

#ensure that political parties are unique
sort $1| uniq -u