# Install CaDiCaL
echo "Setting up CaDiCaL..."
git clone https://github.com/arminbiere/cadical.git
cd cadical && ./configure --competition && make
# Compile leximaxIST library
echo "Compiling leximaxIST library..."
cd ../src/ && make clean release
# Compile packup
echo "Compiling packup..."
cd ../old_packup/ && make clean release
cd ..
echo "Compiling mccs with ILP solver Cbc..."
echo "Installing Cbc..."
mkdir -v cbc
cd cbc
wget https://raw.githubusercontent.com/coin-or/coinbrew/master/coinbrew
chmod u+x coinbrew
./coinbrew fetch Cbc@master
./coinbrew build Cbc
echo "All Done"
