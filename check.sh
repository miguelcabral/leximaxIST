# This is for running the checker on every example. Please change the directory to the correct one in your machine.
rm -f std_err.txt
rm -f std_out.txt
for i in $(seq 1 30)
do
    for j1 in $(seq 1 30)
    do
        for j2 in $(seq 1 30)
        do
            for j3 in $(seq 1 30)
            do
                ./leximax ~/thesis/examples/hard_$i.cnf ~/thesis/examples/f_1_$j1.cnf ~/thesis/examples/f_2_$j2.cnf ~/thesis/examples/f_3_$j3.cnf 2>> std_err.txt >> std_out.txt 
            done
        done
    done
done
