#!/bin/bash 

#check that inputFile exists and we have the right permissions
if [ "$#" -eq 0 ]
    then echo "Give an output file!"
    exit 1
fi
if [ ! -e inputFile.txt ]
    then echo "inputFile doesn't exist"
    exit 1
fi    

if [ ! -r inputFile.txt ]  
    then echo "We don't have the needed permissions for inputFile"
    exit 1
fi

cat inputFile.txt > tmp.txt             #copy inputFile contents in tmp because we shouldn't alter given file

#ensure there are no duplicate votes by sorting and keeping the unique "tuples" of the first two columns (delimiter (t = ' '))
sort -u -t ' ' -k 1,2 -s -o tmp.txt tmp.txt
#get frequency count of values from the third column till the last and store it in the output file given in command line
awk -F ' ' '{for(i=3;i<=NF;i+=1) { printf OFS $i }; print "";}' tmp.txt | uniq -c | sort -nr  > wanted.txt
#remove last line from the final output file because sort creates an empty line and therefore from the previous sorts it returns us a count=1 for no political party (nonsense)
sed '$ d' wanted.txt  > $1
nawk '{sub(/^[ \t]+/," ")};1' $1 > wanted.txt          #remove redundant tabs and white spaces for easier similarity checking with other files
cat wanted.txt > $1
rm tmp.txt
rm wanted.txt
exit 0