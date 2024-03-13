#!/bin/bash 

#check that inputFile exists and we have the right permissions
if [ "$#" -eq 0 ]
    then echo "Give an output file!"
    exit 1
fi
if [ ! -e inputFile ]
    then echo "inputFile doesn't exist"
    exit 1
fi    

if [ ! -r inputFile ]  
    then echo "We don't have the needed permissions for inputFile"
    exit 1
fi

cat inputFile > tmp.txt             #copy inputFile contents in tmp because we shouldn't alter given file

#ensure there are no duplicate votes by sorting and keeping the unique "tuples" of the first two columns (delimiter (t = ' '))
sort -u -t ' ' -k 1,2 -s -o tmp.txt tmp.txt
#keep only political parties from given file
awk -F ' ' '{for(i=3;i<=NF;i+=1) { printf OFS $i }; print "";}' tmp.txt > $1 
#get frequency count for each political party
sort $1 | uniq -c > tmp.txt
#store results in "political Party number of votes" format
awk -F ' ' '{for(i=2;i<=NF;i+=1) { printf OFS $i }; print OFS $1; } ' tmp.txt > $1
#remove redundant spaces and tabs
nawk '{sub(/^[ \t]+/," ")};1' $1 > tmp.txt
cat tmp.txt > $1
#store stats file without last line that refers to total vote (if the two scripts have the same number of votes for each political party then they will have the same total number of votes)
#we need the same format for diff
sed '$ d' stat.txt > copy.txt
diff $1 copy.txt
rm tmp.txt 
rm copy.txt      
exit 0