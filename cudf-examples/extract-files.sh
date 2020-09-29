for f in cudf-examples/*.bz2
do
    bzip2 -d $(basename "$f")
done
