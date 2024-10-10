#!/bin/sh

# start cron
/usr/sbin/crond -b -l 8

/lorareforwarder/build/loraReforwarder