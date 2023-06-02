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
#keep only political parties and store them in the output file given in command line
awk -F ' ' '{for(i=3;i<=NF;i+=1) { printf OFS $i }; print "";}' $1 > pollerResultsFile
#get frequency count for each political party
sort pollerResultsFile | uniq -c > tmp.txt
#store results in "political Party number of votes" format
awk -F ' ' '{for(i=2;i<=NF;i+=1) { printf OFS $i }; print OFS $1; } ' tmp.txt > pollerResultsFile
#get rid of redundant spaces & tabs
nawk '{sub(/^[ \t]+/," ")};1' pollerResultsFile >  tmp.txt
cat tmp.txt > pollerResultsFile
#store stats file without last line that refers to total vote (if the two scripts have the same number of votes for each political party then they will have the same total number of votes too)
#we need the same format for diff
sed '$ d' stat.txt > copy.txt
diff pollerResultsFile copy.txt
rm tmp.txt 
rm copy.txt       
exit 0