#!/bin/bash

#if cmp -s "$file1" "$file2"; then
#    printf 'The file "%s" is the same as "%s"\n' "$file1" "$file2"
#else
#    printf 'The file "%s" is different from "%s"\n' "$file1" "$file2"
#fi

ref=( `ls ref_results` ) # All ref files
tst=( `ls tests` ) # All test files

for ((i = 0 ; i < ${#tst[*]} ; i++)); do # Go over each tst file
	printf 'Check "$s"' "${tst[i]}"
	./bp.exe ${tst[i]} > tmp # TODO: make sure it works (couldn't test it)
	if cmp -s "${ref[i]}" "$tmp"; then #compare the outputs
		printf ' - Pass \n'
	else
		printf ' - Fail \n'
	fi
	#echo ${tst[i]} and ${ref[i]}
done