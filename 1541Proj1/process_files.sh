#!/bin/bash

for btbsize in 32 64 128 
	do
		outputfname="outputs/btb${btbsize}.txt"
		touch $outputfname
		: > $outputfname
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
					./pipeline"${btbsize}" $trace 0 $p_method >> $outputfname 
					echo "#######################################################################" >> $outputfname 
					echo "#######################################################################" >> $outputfname 
				done
		done
done
