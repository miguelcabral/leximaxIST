# prerequisites for running this: install lpsolve, rc2.py, mccs, leximax library and packup. See README.md.
PATHRC2="~/thesis/default-solver/RC2/bin/rc2.py -vv" # change this if necessary 
for f in cudf-examples/*.cudf
do
    TMPFILE=results/p_$(basename $f)_tmp.out
    RESFILE=results/p_$(basename $f).out
    echo "################### PACKUP ####################" > $TMPFILE
    # run packup on f, get the # lines and put them on p_f.out in /results directory
    timeout -s SIGKILL 20m ./old_packup/packup -t --max-sat --leximax --external-solver \
"$PATHRC2" $f >> $TMPFILE 2>> $TMPFILE
    grep '#' $TMPFILE > $RESFILE
done
