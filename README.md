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

## Example of collected dataset

Over the course of more than a year, the software was deployed and a dataset of packets has been stored, available at: https://doi.org/10.5281/zenodo.21457870

## Cite work
Dataset for the paper "Why Not Collect While Forwarding? Transparent LoRaWAN Traffic Logging for Dataset Generation", presented at IEEE SWC 2026.
```
@INPROCEEDINGS{11763803,
  author={Spadaccino, Pietro and Hedfif, Yasmine and Locatelli, Pierluigi and Cattai, Tiziana and Casalbore, Marco and Panella, Massimo and Cuomo, Francesca},
  booktitle={2026 IEEE Smart World Congress (SWC)}, 
  title={Why Not Collect While Forwarding? Transparent LoRaWAN Traffic Logging for Dataset Generation}, 
  year={2026},
  volume={},
  number={},
  pages={47-50},
  doi={10.1109/SWC69865.2026.11763803}}
```


