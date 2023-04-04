# LoRaReforwarder - WIP
LoRaWan UDP Semtech protocol forwarder
Receives packets from a LoRa gateway, store received payloads in a local SQL database and forward them to the specified network server

## Software execution

Software requirements:
- Minimum CMake VERSION 3.20.0
- Pulled submodules in third_party directory

**Building and running commands**

Building the project:
```sh
mkdir build && cd build
cmake ..
make
```

Building and run the project when inside the build folder:
```sh
./loraReforwarder
```