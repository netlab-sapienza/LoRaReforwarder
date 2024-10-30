# LoRaReforwarder
LoRaWan UDP Semtech protocol forwarder

Receives packets from a LoRa gateway, stores received payloads in a local SQL database and forwards them to the specified network server

## Software execution

Requirements:
- MariaDB SQL Database
- Network Server

Software requirements:
- Minimum CMake VERSION 3.20.0
- Pulled submodules in third_party directory

**Building and running commands**

Build the project:
```sh
mkdir build && cd build
cmake ..
make
```

Run the project when inside the build folder:
```sh
./loraReforwarder
```
Rembember to move the .env file to the same folder of the built executable!


**Building and running software using Docker**

Build the project image:
```sh
docker build . -t netlab-sapienza/lorareforwarder
```

Run the project on Docker:
```sh
docker-compose up -d
```

