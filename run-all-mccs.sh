echo "Running mccs with the following parameters:\n\\
\tbenchmarks: $1\n\\
\tuser criteria: $2\n\\
\texternal solver: $3\n" >> /home/mcabral/what-am-i-running.txt
find home/mcabral/data/benchmarks/mancoosi/$1 -type f -exec sh run-mccs.sh $1 $2 $3 $4 '{}' \;
echo "Finnished running mccs with the following parameters:\n\\
\tbenchmarks: $1\n\\
\tuser criteria: $2\n\\
\texternal solver: $3\n" >> /home/mcabral/what-am-i-running.txt
