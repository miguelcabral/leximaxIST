for f in *.cudf
do
    bzip2 -z $(basename "$f")
done
