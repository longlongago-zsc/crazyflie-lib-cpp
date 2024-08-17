# crazyflie-lib-cpp
Like crazyflie-lib-python but written fully in c++

## How to install 

1. `git clone --recursive https://github.com/longlongago-zsc/crazyflie-lib-cpp.git`
2. `cd crazyflie-lib-cpp/crazyflie-link-cpp`
3. `git submodule init -- libusb/`
4. `git submodule update -- libusb/`
5. `cd ..`

## How to compile 
1. `mkdir build && cd build`
2. `cmake ..`
3. `cmake --build .` or just `make`

Run the file you wish with `./<executable_file_name>`
