# prerequisites for running this: install lpsolve, rc2.py, mccs, leximax library and packup. See README.md.
for f in cudf-examples/*.cudf
do
    # run mccs on cudf-file, get the # lines and put them in a file cudf-file.out
    echo "################### MCCS ######################" > "$(basename "$f")_tmp.out"
    ./mccs-1.1/mccs -v1 -i "$f" -leximax[-removed,-notuptodate,-nunsat[recommends:,true],-new] >> "$(basename "$f")_tmp.out" 2>> "$(basename "$f")_tmp.out"
    echo "################### PACKUP ####################" >> "$(basename "$f")_tmp.out"
    # run packup on cudf-file, get the # lines and put append them to cudf-file.out
    ./old_packup/packup -t --max-sat --leximax --external-solver 'rc2.py -vv' "$f" 2>> "$(basename "$f")_tmp.out" >> "$(basename "$f")_tmp.out"
    grep '#' "$(basename "$f")_tmp.out" > "results/$(basename "$f").out"
    rm "$(basename "$f")_tmp.out"
done
