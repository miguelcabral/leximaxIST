# prerequisites for running this: install lpsolve, rc2.py, mccs, leximax library and packup. See README.md.
for f in cudf-examples/*.cudf
do
    # run mccs on f, get the # lines and put them in m_f.out in /results directory
    echo "################### MCCS ######################" > "results/m_$(basename "$f")_tmp.out"
    ./mccs-1.1/mccs -v1 -i "$f" -leximax[-removed,-notuptodate,-nunsat[recommends:,true],-new] >> "results/m_$(basename "$f")_tmp.out" 2>> "results/m_$(basename "$f")_tmp.out"
    grep '#' "results/m_$(basename "$f")_tmp.out" > "results/m_$(basename "$f").out"
    rm "results/m_$(basename "$f")_tmp.out"
done
