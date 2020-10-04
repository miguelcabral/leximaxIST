f=$1 # input cudf file
PATHRC2="~/thesis/default-solver/RC2/bin/rc2.py -vv" # change this if necessary
TMPFILE=results/p_$(basename $f)_tmp.out
RESFILE=results/p_$(basename $f).out
echo "################### PACKUP ####################" > $TMPFILE
# run packup on f, get the # lines and put them on p_f.out in /results directory
timeout 20m -k 1s ./old_packup/packup -t --max-sat --leximax --external-solver \
"$PATHRC2" $f >> $TMPFILE 2>> $TMPFILE
grep '#' $TMPFILE > $RESFILE
rm $TMPFILE
