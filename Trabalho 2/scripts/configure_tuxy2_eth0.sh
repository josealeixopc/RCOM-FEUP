#!/bin/bash

/etc/init.d/networking restart
ifconfig eth0 up
ifconfig eth0 172.16.41.1/24
