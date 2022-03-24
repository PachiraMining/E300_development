#!/bin/bash

#Walet Address
wallet=0x2784685ba4a940406b185f945c26104d64f7562e

#Worker ID
workerID=E300miner

#Temperature Threshold (Celcius)
maxtemp=70

#Pool
pool=$(echo "-P stratum1+tcp://$wallet.$workerID@eth-na.f2pool.com:6688")

#IP
ipaddress=192.168.1.159

#Autotune Phase
#phase = 1: use best frequencies from text file. If there is no text file, start autotuning
 
#phase = 4: use frequencies from command line
phase=4

sudo  ./ethminer --fpga -a 530 -b 312 -c 125 -q 100 -e 4 -f -d 70  -P stratum1+tcp://0x2784685ba4a940406b185f945c26104d64f7562e.vu35p@eth-na.f2pool.com:6688  --ip 192.168.1.62

while true; do
			sudo killall ethminer
			sudo  ./ethminer --fpga -a 510 -b 312 -c 125 -q 110 -e $phase -f -d $maxtemp $pool --ip $ipaddress
			echo "Restarting the miner in $timeretry seconds..."
			sleep 10
done