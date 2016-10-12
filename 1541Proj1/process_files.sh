#!/bin/bash

outputfname="outputs/trace_output.txt"
touch $outputfname
: > $outputfname

for btbsize in 32 64 128 
	do
		for trace in inputs/* 
		do
			fname=${trace##*/}
			fname=${fname%.tr}
			for p_method in 0 1 2
				do
					echo "#######################################################################" >> $outputfname 
					echo "#######################################################################" >> $outputfname 
					echo "File ${fname}" >> $outputfname
		   		 	echo "Method ${p_method}" >> $outputfname
					echo "BTB Size ${btbsize}" >> $outputfname
					./pipeline $trace $p_method 0 $btbsize >> $outputfname 
					echo "#######################################################################" >> $outputfname 
					echo "#######################################################################" >> $outputfname 
				done
		done
done
