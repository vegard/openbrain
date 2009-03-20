set -e
set -u

mkdir -p png

g++ -Wall -g -IChipmunk-4.1.0/src/ -I/usr/include/SDL -o play play-main.cc -LChipmunk-4.1.0/src/ -lchipmunk -lSDL -lGL -lGLU -lpng
./play
