#!/bin/bash
# searches src folder for all header files
# prints all unique directory paths

arr=( $(find src/ -name '*.h') ) # use find command and store each result as an element in an array

# loop through array remove file name (keep directory path)
for ((i=0; i<${#arr[@]}; i++));
do
	arr[$i]=$(dirname "${arr[$i]}")
done

# extract unique values from array
sorted_unique=`tr ' ' '\n' <<< "${arr[@]}" | sort -u | tr '\n' ' '`

# loop through unique values and print one per line
for i in ${sorted_unique[@]}; 
do
	#echo "'"'-I'"'", "'"$i"'"
	echo '-I'$i
done
