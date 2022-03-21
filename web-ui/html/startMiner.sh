#!/bin/bash

#Walet Address
wallet=0x2784685ba4a940406b185f945c26104d64f7562e

#Worker ID
workerID=dracaena

#Number of FPGAs
quantity=3

#Temperature Threshold (Celcius)
maxtemp=68

#Pool
pool=$(echo "-P stratum1+tcp://$wallet.$workerID@eth.f2pool.com:6688")
#pool=$(echo "-P stratum+tcp://duongduc@eth.ss.poolin.com:443")

#Autotune Phase (please dont change this. Default: 3)
phase=1

#sudo ./ethminer --fpga -a 475 -b 285 -c 120 -q 105 -e $phase -f -n $quantity -d $maxtemp  $pool
sudo ./ethminer --fpga -a 530 -b 322 -c 125 -q 110 -e $phase -f -n $quantity -d $maxtemp  $pool
