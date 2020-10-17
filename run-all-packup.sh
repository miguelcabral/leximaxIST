echo "Running packup with the following parameters:" >> /home/mcabral/what-am-i-running.txt
echo "\tbenchmarks: $1" >> /home/mcabral/what-am-i-running.txt
echo "\tuser criteria: $2" >> /home/mcabral/what-am-i-running.txt
echo "\texternal solver: $4" >> /home/mcabral/what-am-i-running.txt
echo "\tmax sat/pbo: $5" >> /home/mcabral/what-am-i-running.txt
find /home/mcabral/data/benchmarks/mancoosi/$1 -type f -exec sh run-packup.sh $1 $2 $3 $4 $5 '{}' \;
flock /home/mcabral/what-am-i-running.txt sh packup-is-done.sh $1 $2 $3 $4 $5

