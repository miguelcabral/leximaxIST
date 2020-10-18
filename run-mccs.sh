# BENCHMARK is provided by the input: misc-live, misc2010, misc2011, misc2012, ...
BENCHMARK=$1
# CRITERION is the user criterion for optimisation:
CRITERION=$2
# SOLVERNAME is the name of the solver
SOLVERNAME=$3
# INSTANCE is the input cudf instance
INSTANCE=$4
# SOLUTION contains the solution of the problem; this can be checked using the solution checker
SOLUTION=/home/mcabral/data/tmp/$BENCHMARK/m_"$SOLVERNAME"_$(basename $INSTANCE).sol
if ! [ -f "$SOLUTION" ]; then
    touch $SOLUTION
    # STATS contains the measurements taken by runsolver during the execution
    STATS=/home/mcabral/data/tmp/$BENCHMARK/m_"$SOLVERNAME"_$(basename $INSTANCE).stats
    # run mccs
    /home/mcabral/runsolver/src/runsolver --vsize-limit 4000 -C 1200 -d 10 -w $STATS -o $SOLUTION \
/home/mcabral/thesis/mccs-1.1/mccs -v1 -i $INSTANCE -leximax[$CRITERION]
fi
