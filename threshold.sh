#!/bin/bash

# cmd_threshold shape length errors method
function cmd_threshold
{
    if [[ $4 = 'ilp' ]]
    then
        CMD="python $TSV/ilp/threshold.py $2 $3 $1"
    else
        CMD="$BIN/demo_threshold $1 $2 $3 -a $4"
    fi
}

# exec_threshold shapes.tsv filename.tsv
function exec_threshold
{
    shapes=$1
    filename=$2

    if [[ ! -e $filename ]]; then
        echo -e "length\terrors\tshape\tmethod\tthreshold\tseconds" > $filename
    fi

    length=$LENGTH
    errors=$ERRORS
    for shape in $SHAPES;
    do
        for method in lemma apx ilp exact;
        do
            cmd_threshold $shape $length $errors $method
            echo $CMD
            output=$($CMD)
            if [ $? -eq 0 ]; then
                echo -e "$length\t$errors\t$shape\t$method\t$output" >> $filename
            fi
        done
    done
}

# ======================================================================================================================

if [ ! $# -eq 1 ]
then
    exit
fi

export PYTHONPATH=/opt/ibm/ILOG/CPLEX_Studio_Preview1261/cplex/python/2.6/x86-64_linux/

BIN=~/Code/seqan-builds/Release-Gcc/bin
TSV=~/Datasets/threshold/

LENGTH=100
SHAPES=$(cat $1)
#ERRORS=8

for ERRORS in 8 9 10
do
    exec_threshold "${SHAPES}" $TSV/threshold.tsv
done
