echo "Running mccs with the following parameters:" >> /home/mcabral/what-am-i-running.txt
echo "\tbenchmarks: $1" >> /home/mcabral/what-am-i-running.txt
echo "\tuser criteria: $2" >> /home/mcabral/what-am-i-running.txt
echo "\texternal solver: $3" >> /home/mcabral/what-am-i-running.txt
find /home/mcabral/data/benchmarks/mancoosi/$1 -type f -exec sh run-mccs.sh $1 $2 $3 $4 '{}' \;
flock /home/mcabral/what-am-i-running.txt sh mccs-is-done.sh $1 $2 $3

