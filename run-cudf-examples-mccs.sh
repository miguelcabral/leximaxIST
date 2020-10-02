# prerequisites for running this: install lpsolve, rc2.py, mccs, leximax library and packup. See README.md.
for f in cudf-examples/*.cudf
do
    TMPFILE="results/m_$(basename "$f")_tmp.out"
    RESFILE="results/m_$(basename "$f").out"
    # run mccs on f, get the # lines and put them in m_f.out in /results directory
    echo "################### MCCS ######################" > $TMPFILE
    timeout 20m ./mccs-1.1/mccs -v1 -i "$f" \
-leximax[-removed,-notuptodate,-nunsat[recommends:,true],-new] >> $TMPFILE 2>> $TMPFILE
    grep '#' $TMPFILE > $RESFILE
    rm $TMPFILE
done
