#!/bin/bash

#if cmp -s "$file1" "$file2"; then
#    printf 'The file "%s" is the same as "%s"\n' "$file1" "$file2"
#else
#    printf 'The file "%s" is different from "%s"\n' "$file1" "$file2"
#fi

ref=( `ls ref_results` ) # All ref files
tst=( `ls tests` ) # All test files

for ((i = 0 ; i < ${#tst[*]} ; i++)); do # Go over each tst file
	#printf 'Check "$s"' "${tst[i]}"
	./cmake-build-debug/CA-hw1.exe  tst/${tst[i]} > tmp # TODO: Change to your true exe
	if cmp -s "ref_results/${ref[i]}" "$tmp"; then #compare the outputs
		echo "Test ${tst[i]} - Pass"
	else
		echo "Test ${tst[i]} - Fail"
	fi
	#echo ${tst[i]} and ${ref[i]}
done
