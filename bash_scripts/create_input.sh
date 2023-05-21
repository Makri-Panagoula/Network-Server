#!/bin/bash 

#check validity of command line arguments
if [ "$#" -le 1 ]
    then echo "Give 2 command line arguments!"
    exit 1
fi
if [ ! -e $1 ]
    then echo "Given file doesn't exist!"
    exit 1
fi    
if [ $2 -le 0 ]
    then echo "numLines should be positive!"
    exit 1
fi
if [ ! -r $1 ]  
    then echo "We don't have the needed permissions!"
    exit 1
fi

sort  -u -o $1 $1        #ensure that political parties are unique
touch inputFile.txt      #create wanted file
let "lines = $(wc -l < "$1")"       

for(( i = 0 ; i < $2 ; i++))
do
    let "name_len = $RANDOM % 10 + 3" #pick a random size for name & surname in the range of [3,12]
    let "sur_len = $RANDOM % 10 + 3"   
    let "line_num = $RANDOM % lines + 1"            #make sure the line number we are taking is in an acceptable range
    line=$(head -n $line_num  "$1" | tail -1)       #read line from file

    #name and surname should start with a capital letter and the rest should be small letters
    initial_n=$(tr -dc A-Z </dev/urandom | head -c 1)
    initial_s=$(tr -dc A-Z </dev/urandom | head -c 1)

    let "num_r = $name_len - 1"
    let "sur_r = $sur_len - 1"

    rest_n=$(tr -dc a-z </dev/urandom | head -c $num_r)
    rest_s=$(tr -dc a-z </dev/urandom | head -c $sur_r)

    name="$initial_n$rest_n"
    surname="$initial_s$rest_s"
    to_write="$name $surname $line"             #concatenate them all in a string and store  in file
    echo "$to_write" >> inputFile.txt              
   
done

exit 0