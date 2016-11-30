#!/bin/bash

/etc/init.d/networking restart
ifconfig eth0 up
ifconfig eth0 172.16.40.1/24

