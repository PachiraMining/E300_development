# 1. Load bitstream into FPGA (Load from vivado)

step 1: Open vivado lab

step 2: Click "Open Hardware Manager"

step 3: Right click "auto connection"

step 4: Right click "local host" => "Add Xilinx Virtual Cable (XVC)"

step 5: update E300 Box's IP address and port  (FPGA1 : port 2541 ; FPGA2: port 2542; FPGA3: port 2543)

step 6: Right click "vu35p/xcu50..." => "Program device" => linking to bitstream file => "program"


# 2. Running miner (Run on ubuntu machine)
There are two options for run miner

Option 1: Run miner by command line
```bash
	sudo  ./ethminer --fpga -a 530 -b 312 -c 125 -q 100 -e 4 -f -d 60  -P stratum1+tcp://0x2784685ba4a940406b185f945c26104d64f7562e.vu35p@eth-na.f2pool.com:6688  ---ip 192.168.1.62
```
_Note_ 
 
 ` -e 1 : Mode auto-tunning mode (Miner will find the best frequency and run it)`

 ` -e 4 : Mode manual  (Miner will receive input clock parameters from command line 
 ` --ip : Your e300 box IP address


Option 2: Run miner by script
```bash
	sudo  ./startMiner.sh
```

