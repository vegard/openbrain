set -e
set -u

g++ -Wall -g -IChipmunk-4.1.0/src/ -o evolve evolve-main.cc -LChipmunk-4.1.0/src/ -lchipmunk
./evolve
