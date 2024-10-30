#!/bin/sh

mkdir -p /lorareforwarder/backups/
if [ -f /lorareforwarder/backups/backup.sql ]; then
    mv /lorareforwarder/backups/backup.sql /lorareforwarder/backups/backup.sql.old
fi
mysqldump -ulorareforwarder -plorareforwarderpwd -h 192.168.135.2 lorareforwarder > "/lorareforwarder/backups/backup.sql"
