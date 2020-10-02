# prerequisites for running this: install lpsolve, rc2.py, mccs, leximax library and packup. See README.md.
# this script is to put all examples running at the same time
for f in cudf-examples/*.cudf
do
    # run mccs on f, get the # lines and put them in m_f.out in /results directory
    (nohup sh run-mccs.sh "$f") &
    # run packup on f, get the # lines and put them on p_f.out in /results directory
    (nohup sh run-packup.sh "$f") &
done
