# This is for running the checker on every example. Please change the directory to the correct one in your machine.
for i in $(seq 2 5)
do
    for hard in ~/thesis/examples/hard_$i\_*
    do
        ./leximax $hard ~/thesis/examples/f_$i.cnf | ./lingeling -q | ./check_sorting $i >> check_output.txt
    done
done
