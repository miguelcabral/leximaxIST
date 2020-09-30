# prerequisites for running this: install lpsolve, rc2.py, mccs, leximax library and packup. See README.md.
for f in cudf-examples/*.cudf
do
#MCCSTMP="m_$(basename "$f")_tmp.out"
# run mccs on cudf-file, get the # lines and put them in a file cudf-file.out
(nohup \
echo "################### MCCS ######################" > "results/m_$(basename "$f")_tmp.out"; \
./mccs-1.1/mccs -v1 -i "$f" -leximax[-removed,-notuptodate,-nunsat[recommends:,true],-new] \
>> "results/m_$(basename "$f")_tmp.out" 2>> "results/m_$(basename "$f")_tmp.out"; \
grep '#' "results/m_$(basename "$f")_tmp.out" > "results/m_$(basename "$f").out"; \
rm "results/m_$(basename "$f")_tmp.out";) \
&
(nohup \
echo "################### PACKUP ####################" >> "results/p_$(basename "$f")_tmp.out"; \
# run packup on cudf-file, get the # lines and put append them to cudf-file.out
./old_packup/packup -t --max-sat --leximax --external-solver 'rc2.py -vv' "$f" \
2>> "results/p_$(basename "$f")_tmp.out" >> "results/p_$(basename "$f")_tmp.out"; \
grep '#' "results/p_$(basename "$f")_tmp.out" > "results/p_$(basename "$f").out"; \
rm "results/p_$(basename "$f")_tmp.out";) \
&
done
