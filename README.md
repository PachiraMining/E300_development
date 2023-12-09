# E300_development
This is the development document for the E300 device

# 1. API List
API: getMiningSatus

Commandline:

	curl --header "Content-Type: application/json" --request GET http://192.168.5.3:8200/controller/getMiningSatus
 
Description: getMiningSatus returns mining status


API: setMiningAction

Commandline:

	curl --header "Content-Type: application/json" --request POST --data '{"action":1}' http://192.168.5.3:8200/controller/runMiningAction"
 
Description: setMiningAction supports Start/Stop mining ("action":1 => start mining ; "action":0 => Stop mining)

API: setVoltage

Commandline:

	curl --header "Content-Type: application/json" --request POST --data '{"voltage_vccint":600,"voltage_hbm":1050,"boardId":3}' http://192.168.5.3:8200/controller/setVoltage
 
Description: setVoltage supports setVccInt, vccHBM. 

			 boardId: 0 => set FPGA0 only
    
			 boardId: 1 => set FPGA1 only
    
			 boardId: 2 => set FPGA2 only
    
			 boardId: 3 => set ALL FPGAs.

API: setTempMax 

Commandline: 

	curl --header "Content-Type: application/json" --request POST --data '{"tempMax":70,"boardId":0}' http://192.168.5.3:8200/controller/setTempMax
 
Description: setTempMax supports set maximum temperaturer of FPGA

 			 boardId: 0 => set FPGA0 only
     
			 boardId: 1 => set FPGA1 only
    
			 boardId: 2 => set FPGA2 only
    
			 boardId: 3 => set ALL FPGAs.

API: getAllInfo

Commandline:

	curl --header "Content-Type: application/json" --request GET http://192.168.5.3:8200/controller/getAllInfo
 
Description: Get ALL information relating to FPGA configuration


API: setOtaUpdate

Commandline:

	curl --header "Content-Type: application/json" --request POST --data {"otaUpdateEnable":1} http://192.168.5.3:8500/firmware/setOtaUpdate
 
Description: Enable/Disable OTA update


API: getOtaUpdate

Commandline:

 	curl --header "Content-Type: application/json" --request GET http://192.168.5.3:8500/firmware/getOtaUpdate
  
Description: Get firmware version and OTA status

 
API: setFanMode

Commandline:

	curl --header "Content-Type: application/json" --request POST --data '{"fan_mode":1,"fan_level":10}' http://192.168.5.3:8200/controller/setFansMode
 
Description: API supports set fans configuration 

			 fan_mode : 0 -> auto mode (you may ignore fan_level)
    
			 fan_mode : 1 -> manual mode, you need to set fan_level (fan_level ranges from 1 to 10)


API: getAllFanInfo

Commandline:

	curl --header "Content-Type: application/json" --request GET http://192.168.5.3:8200/controller/getAllFanInfo
 
Description: get fans information

API: Chnage Jtag clock API

Commandline:

	curl --header "Content-Type: application/json" --request POST --data '{"ratio":15,"boardId":3}' http://192.168.5.3:8200/controller/setJtagClock
 
Description: change Jtag clock

 			 boardId: 0 => set FPGA0 only
     
			 boardId: 1 => set FPGA1 only
    
			 boardId: 2 => set FPGA2 only
    
			 boardId: 3 => set ALL FPGAs.

    			 ratio ranges 1 to 15
	
       			 Jtag_clock = 100 / ratio
	   		(VU13P boards should set ratio = 10 before running kylacoin)


API: getDracaMinerStatus

Commandline:

	curl -X POST -d 'Page=getDracaMinerStatus' -H 'Content-Type: application/x-www-form-urlencoded' http://192.168.5.3/cgi-bin/qcmap_web_cgi.cgi

Response result

{"Page":"getDracaMinerStatus", "status":"4", "developer":"wf", "enabled":"1", "info": {"running_mode":"0","a":"radiant", "o":"stratum+tcp://usse.vipor.net:5067", "u":"12XfbWYep8zKj9RdmfRiwL3fu3PRvNRdfe", "miner":"vu33box110", "fpga_clk_core":"450"}
, "result":"SUCCESS"}

Explain result:

"status" : Status of developer
	 - 1: Running TheAllFather bitstream
	 - 2: Running Teamredminer bitstream
	 - 3: Running K1 Lab bitstream
	 - 4: Running WhiteFire bitstream
	 - 0: Stopped

"developer" : developer name
     - wf : WhiteFire
     - aft : TheAllFather
     - k1  : k1 Lab
     - trm : Teamredminer

"enabled" :
 	 0 - disabled auto start mining
 	 1 - enabled auto start mining

""info": mining info

     running_mode : running mode 
        0 - menu running_mode
        1 - command line running_mode

     a  : algorithm
     	radiant 
     	kylacoin 
     	etchash
     	ironfish
     	kas

     u : wallet address
     miner : workerID
     fpga_clk_core: mINING CLOCK

# 2. Firmware on zynq board
- Sample projects to use communication features between zynq board and FPGA via I2C, UART, jtag,... to read temperature, voltage, load bitstream,...
# 3. hardware
- there is structure diagram of E300, schematic, part number, and constrain configuration setting.
- the public manual describes the connection between the zynq board and the FPGA, the user manual, the instruction set, the peripheral,...

# 4. Web-UI
- Describe the features on the E300 such as control, configuration and monitoring
