for f in *.bz2
do
    echo "Extracting $f"
    bzip2 -d $(basename "$f")
done
