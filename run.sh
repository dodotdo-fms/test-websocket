mkdir build
cd build
cmake ../libwebsockets -DLWS_WITH_SSL=OFF
make
rm -rf CMake* Makefile

cmake ../src
make

