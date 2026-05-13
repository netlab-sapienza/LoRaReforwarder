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

## Deployment Topology

The system is designed with a distributed architecture where the LoRa forwarder runs on each gateway and connects to a central database hosted on the network server. This allows multiple gateways to share a single MariaDB instance.

### Architecture

```
┌─────────────────────────────┐       ┌────────────────────────────────────────┐
│        Gateway N            │       │         Network Server                 │
│                             │       │                                        │
│  ┌───────────────────────┐  │       │  ┌──────────────┐   ┌───────────────┐  │
│  │ lorareforwarder       │  │       │  │   MariaDB    │   │  phpMyAdmin   │  │
│  │  (UDP :1700)          │──┼───────┼─►│   Server     │───┼─► (port XXXX)│  │
│  │                       │  │       │  │              │   │               │  │
│  │  .env configured:     │  │       │  │              │   │               │  │
│  │  - MARIADB_SERVER_ADDR│  │       │  │              │   │               │  │
│  │    = <server_ip>      │  │       │  └──────────────┘   └───────────────┘  │
│  │  - NETWORK_SERVER_ADDR│  │       │                                        │
│  │    = <network_server> │  │       │  Shared network: X.Y.Z.0/24            │
│  │  - MARIADB_SERVER_PORT│  │       │                                        │
│  │    = 1700             │  │       │                                        │
│  │  - SERVER_PORT        │  │       │                                        │
│  │    = 1700             │  │       │                                        │
│  └───────────────────────┘  │       └────────────────────────────────────────┘
└─────────────────────────────┘
```

Each gateway runs the `lorareforwarder` container, which:
1. Receives UDP packets from the LoRa gateway on port 1700 (Semtech UDP protocol)
2. Forwards those packets to the Network Server (e.g., The Things Network)
3. Stores received packets in the central MariaDB database on the network server

### Setting up the central database (Network Server)

Deploy MariaDB and phpMyAdmin on the network server using `docker-compose.yml`. By default, MariaDB is assigned the IP `192.168.135.2` on the shared bridge network:

```sh
docker-compose up -d mariadb phpmyadmin
```

The `init.sql` script automatically creates the `lorareforwarder` database, the required tables (`upPackets`, `downPackets`), and grants access to gateway containers.

### Setting up a gateway

On each gateway:

1. Build (or pull) the `lorareforwarder` image:
   ```sh
   docker build . -t netlab-sapienza/lorareforwarder
   # or
   docker pull netlab-sapienza/lorareforwarder
   ```

2. Configure `.env` with the correct values for your setup. The key variables are:

   | Variable | Description | Example |
   |---|---|---|
   | `SERVER_PORT` | UDP port to listen on (gateway → forwarder) | `1700` |
   | `MARIADB_SERVER_ADDR` | IP address of the central MariaDB server | `192.168.135.2` |
   | `MARIADB_SERVER_PORT` | MariaDB port | `3306` |
   | `MARIADB_SERVER_DBNAME` | Database name | `lorareforwarder` |
   | `MARIADB_SERVER_USERNAME` | Database username | `lorareforwarder` |
   | `MARIADB_SERVER_PWD` | Database password | `lorareforwarderpwd` |
   | `NETWORK_SERVER_ADDR` | Network Server address (e.g., TTN) | `eu1.cloud.thethings.network` |
   | `NETWORK_SERVER_PORT` | Network Server UDP port | `1700` |
   | `DEBUG` | Enable debug logging | `false` |

3. Run the container:
   ```sh
   docker run -d \
     --name lorareforwarder \
     --net host \
     -p 1700:1700/udp \
     -v /opt/lorareforwarderBackups:/lorareforwarder/backups \
     --restart always \
     --env-file .env \
     netlab-sapienza/lorareforwarder:latest
   ```

   Using `--net host` allows the container to bind directly to port 1700 UDP. Alternatively, use the bridge network approach from `docker-compose.yml` with a static IP:

   ```yaml
   lorareforwarder:
     image: netlab-sapienza/lorareforwarder:latest
     ports:
       - "1700:1700/udp"
     networks:
       database_network:
         ipv4_address: 192.168.135.<N>
     restart: always
     volumes:
       - /opt/lorareforwarderBackups:/lorareforwarder/backups
     env_file: .env
   ```

### Packet flow

1. The LoRa gateway sends packets via UDP (Semtech UDP protocol) to the `lorareforwarder` container on the gateway (port 1700)
2. The `lorareforwarder` parses the packets and:
   - Forwards them to the Network Server via UDP
   - Stores `PKT_PUSH_DATA` packets in the `upPackets` table and `PKT_PULL_RESP` packets in the `downPackets` table on the central MariaDB
   - Sends responses from the Network Server back to the gateway
3. The database can be inspected via phpMyAdmin on the network server
