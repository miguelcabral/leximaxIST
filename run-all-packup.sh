echo "Running packup with the following parameters:\n\\
\tbenchmarks: $1\n\\
\tuser criteria: $2\n\\
\texternal solver: $4\n\\
\tmax sat/pbo: $5\n" >> /home/mcabral/what-am-i-running.txt
find home/mcabral/data/benchmarks/mancoosi/$1 -type f -exec sh run-packup.sh $1 $2 $3 $4 $5 '{}' \;
echo "Finnished running packup with the following parameters:\n\\
\tbenchmarks: $1\n\\
\tuser criteria: $2\n\\
\texternal solver: $4\n\\
\tmax sat/pbo: $5\n" >> /home/mcabral/what-am-i-running.txt
