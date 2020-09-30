# prerequisites for running this: install lpsolve, rc2.py, mccs, leximax library and packup. See README.md.
for f in cudf-examples/*.cudf
do	
    echo "################### PACKUP ####################" > "p_$(basename "$f")_tmp.out"
    # run packup on f, get the # lines and put them on p_f.out in /results directory
    ./old_packup/packup -t --max-sat --leximax --external-solver 'home/mcabral/thesis/default-solver/RC2/bin/rc2.py -vv' "$f" 2>> "p_$(basename "$f")_tmp.out" >> "p_$(basename "$f")_tmp.out"
    grep '#' "p_$(basename "$f")_tmp.out" > "results/p_$(basename "$f").out"
    rm "p_$(basename "$f")_tmp.out"
done
