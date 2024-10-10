#!/bin/sh

mkdir -p /lorareforwarder/backups/$(date +"%Y")
mysqldump -ulorareforwarder -plorareforwarderpwd -h 192.168.135.2 lorareforwarder > "/lorareforwarder/backups/$(date +"%Y")/$(date +"%Y%m%dT%H%M").sql"
