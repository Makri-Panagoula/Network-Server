#!/bin/bash 
#check that poll-log exists and we have the right permissions
if [ "$#" -eq 0 ]
    then echo "Give the poll-log file!"
    exit 1
fi

if [ ! -e  $1 ]
    then echo "Poll-log file doesn't exist!"
    exit 1
fi    

if [ ! -r $1 ]  
    then echo "We don't have the needed permissions for poll-log file!"
    exit 1
fi

#ensure there are no duplicate votes by sorting and keeping the unique "tuples" of the first two columns (delimiter (t = ' '))
sort -u -t ' ' -k 1,2 -s -o $1 $1 
#get frequency count of values from the third column till the last and store it in the output file given in command line
awk -F ' ' '{for(i=3;i<=NF;i+=1) { printf OFS $i }; print "";}' $1 > pollerResultsFile
sort pollerResultsFile | uniq -c > tmp.txt
#keep the same output format as in tallyResults for similarity checking
nawk '{sub(/^[ \t]+/," ")};1' tmp.txt >  pollerResultsFile
rm tmp.txt       
exit 0