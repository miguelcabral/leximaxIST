# BENCHMARK is provided by the input: misc-live, misc2010, misc2011, misc2012, ...
BENCHMARK=$1
# CRITERION is the user criterion for optimisation:
CRITERION=$2
# SOLVERNAME is the name of the solver
SOLVERNAME=$3
# INSTANCE is the input cudf instance
INSTANCE=$4
# SOLUTION contains the solution of the problem; this can be checked using the solution checker
SOLUTION=/home/mcabral/data/tmp/$BENCHMARK/m_$SOLVERNAME_$(basename $INSTANCE).sol
if ! [ -f "$SOLUTION" ]; then
    touch $SOLUTION
    # INFO contains the time measured by packup and the objective vector
    INFO=/home/mcabral/data/tmp/$BENCHMARK/m_$SOLVERNAME_$(basename $INSTANCE).info
    # STATS contains the measurements taken by runsolver during the execution
    STATS=/home/mcabral/data/tmp/$BENCHMARK/m_$SOLVERNAME_$(basename $INSTANCE).stats
    # run mccs
    ./home/mcabral/runsolver/src/runsolver -V 4000 -C 1200 -d 10 -w $STATS \
./home/mcabral/thesis/mccs-1.1/mccs -v1 -i $INSTANCE -o $SOLUTION -leximax[$CRITERION] 2> $INFO
fi
