# BENCHMARK is provided by the input: misc-live, misc2010, misc2011, misc2012, ...
BENCHMARK=$1
# CRITERION is the user criterion for optimisation:
CRITERION=$2
# SOLVERCMD is the solver command
SOLVERCMD=$3
# SOLVERNAME is the name of the solver
SOLVERNAME=$4
# MAXSAT is a string that tells the solver whether to solve MaxSAT or PBO
MAXSAT=$5
# INSTANCE is the input cudf instance
INSTANCE=$6
# SOLUTION contains the solution of the problem; this can be checked using the solution checker
SOLUTION=/home/mcabral/data/tmp/$BENCHMARK/p_$SOLVERNAME_$(basename $INSTANCE).sol
if ! [ -f "$SOLUTION" ]; then
    touch $SOLUTION
    # INFO contains the time measured by packup and the objective vector
    INFO=/home/mcabral/data/tmp/$BENCHMARK/p_$SOLVERNAME_$(basename $INSTANCE).info
    # STATS contains the measurements taken by runsolver during the execution
    STATS=/home/mcabral/data/tmp/$BENCHMARK/p_$SOLVERNAME_$(basename $INSTANCE).stats
    # run packup
    if [ "$MAXSAT" = "max-sat" ]; then
        /home/mcabral/runsolver/src/runsolver --vsize-limit 4000 -C 1200 -d 10 -w $STATS \
/home/mcabral/thesis/old_packup/packup -u "$CRITERION" --max-sat --leximax \
--external-solver "$SOLVERCMD" $INSTANCE $SOLUTION 2> $INFO
    else
        /home/mcabral/runsolver/src/runsolver --vsize-limit 4000 -C 1200 -d 10 -w $STATS \
/home/mcabral/thesis/old_packup/packup -u "$CRITERION" --leximax \
--external-solver "$SOLVERCMD" $INSTANCE $SOLUTION 2> $INFO
    fi
fi
