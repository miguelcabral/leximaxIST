for f in *.bz2
do
    bzip2 -d $(basename "$f")
done
