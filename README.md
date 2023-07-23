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


# 2. Firmware on zynq board
- Sample projects to use communication features between zynq board and FPGA via I2C, UART, jtag,... to read temperature, voltage, load bitstream,...
# 3. hardware
- there is structure diagram of E300, schematic, part number, and constrain configuration setting.
- the public manual describes the connection between the zynq board and the FPGA, the user manual, the instruction set, the peripheral,...

# 4. Web-UI
- Describe the features on the E300 such as control, configuration and monitoring
