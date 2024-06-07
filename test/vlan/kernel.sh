#!/bin/bash

ip link add link tap10 name tap10.10 type vlan id 10
ip link set tap10.10 up
ip addr add 192.168.1.2/24 dev tap10.10
