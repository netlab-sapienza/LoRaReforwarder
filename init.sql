CREATE DATABASE IF NOT EXISTS lorareforwarder;
GRANT ALL PRIVILEGES ON lorareforwarder.* TO 'lorareforwarder'@'192.168.135.3' identified by 'lorareforwarderpwd';
FLUSH PRIVILEGES;
USE lorareforwarder;

CREATE TABLE `downPackets` (
  `id` int(11) PRIMARY KEY AUTO_INCREMENT,
  `imme` tinyint(1) DEFAULT NULL,
  `tmst` bigint(20) DEFAULT NULL,
  `tmms` bigint(20) DEFAULT NULL,
  `freq` float DEFAULT NULL,
  `rfch` int(11) DEFAULT NULL,
  `powe` int(11) DEFAULT NULL,
  `modu` varchar(4) DEFAULT NULL,
  `datr` varchar(100) DEFAULT NULL,
  `codr` varchar(100) DEFAULT NULL,
  `fdev` int(11) DEFAULT NULL,
  `ipol` tinyint(1) DEFAULT NULL,
  `prea` int(11) DEFAULT NULL,
  `size` int(11) DEFAULT NULL,
  `data` text DEFAULT NULL,
  `ncrc` tinyint(1) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE `upPackets` (
  `id` int(11) PRIMARY KEY AUTO_INCREMENT,
  `gwui` varchar(18) DEFAULT NULL,
  `time` varchar(100) DEFAULT NULL,
  `tmms` bigint(20) DEFAULT NULL,
  `tmst` int(11) DEFAULT NULL,
  `freq` int(11) DEFAULT NULL,
  `chan` int(11) DEFAULT NULL,
  `rfch` int(11) DEFAULT NULL,
  `stat` tinyint(4) DEFAULT NULL,
  `modu` varchar(4) DEFAULT NULL,
  `datr` varchar(100) DEFAULT NULL,
  `codr` varchar(100) DEFAULT NULL,
  `rssi` int(11) DEFAULT NULL,
  `lsnr` float DEFAULT NULL,
  `size` int(11) DEFAULT NULL,
  `data` text DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;