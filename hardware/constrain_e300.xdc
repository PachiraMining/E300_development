##########################################################################
## FPGA information
## Product family: Virtex UltraScale+ HBM
## Part number: xcvu35p_CIV-fsvh2104-2-e
## Please use Vivado/Vivado Lab 2020.2.2 or newer version and install Virtex UltraScale+ HBM Device package.
##########################################################################


set_property CONFIG_VOLTAGE 1.8 [current_design]
#

set_property IOSTANDARD LVCMOS12 [get_ports rx]
set_property PACKAGE_PIN C12 [get_ports rx]
set_property IOSTANDARD LVCMOS12 [get_ports tx]
set_property PACKAGE_PIN B9 [get_ports tx]

# use ref clk 2
set_property PACKAGE_PIN BB18 [get_ports clk_p]
set_property PACKAGE_PIN BC18 [get_ports clk_n]
set_property IOSTANDARD LVDS [get_ports clk_p]
set_property IOSTANDARD LVDS [get_ports clk_n]
set_property DIFF_TERM_ADV TERM_100 [get_ports clk_p]
set_property DIFF_TERM_ADV TERM_100 [get_ports clk_n]
#
create_clock -period 10.000 -name clk_p [get_ports clk_p]

create_clock -period 4.000 -name clk_hash -waveform {0.000 2.000} [get_pins regs_file/clock_generator/BUFGCE_inst1/O]
create_clock -period 2.000 -name clk_axi -waveform {0.000 1.000} [get_pins regs_file/clock_generator/BUFGCE_inst2/O]
create_clock -period 10.000 -name clk_hbm -waveform {0.000 5.000} [get_pins regs_file/clock_generator/BUFGCE_inst3/O]

set_false_path -from [get_pins regs_file/registers_reg*/C]
set_false_path -from [get_pins eth_main_loop_inst/hash_rate_reg*/C]

#set_property BITSTREAM.ENCRYPTION.ENCRYPTKEYSELECT EFUSE [current_design]
#set_property BITSTREAM.ENCRYPTION.KEYFILE /home/marvin/Desktop/key.nky [current_design]
#set_property BITSTREAM.ENCRYPTION.ENCRYPT YES [current_design]

set_false_path -from [get_pins MemoryController_inst/u_hbm_0/inst/TWO_STACK.u_hbm_top/TWO_STACK_HBM.u_hbm_temp_rd_0/temp_value_r_reg*/C]
set_false_path -from [get_pins MemoryController_inst/u_hbm_0/inst/TWO_STACK.u_hbm_top/TWO_STACK_HBM.u_hbm_temp_rd_1/temp_value_r_reg*/C]

set_false_path -from [get_pins DAGgenController_inst/finish_reg/C]

set_property CLOCK_DEDICATED_ROUTE BACKBONE [get_nets clk_input_inst/inst/clk_out1]
set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets clk_input_inst/inst/clkin1_ibufds/O]

set_false_path -from [get_clocks *clk_hash*] -to [get_clocks *clk_out1_clk_input*]
set_false_path -from [get_clocks *clk_out1_clk_input*] -to [get_clocks *clk_hash*]

set_false_path -from [get_clocks *clk_axi*] -to [get_clocks *clk_out1_clk_input*]
set_false_path -from [get_clocks *clk_out1_clk_input*] -to [get_clocks *clk_axi*]

set_false_path -from [get_clocks *clk_hbm*] -to [get_clocks *clk_out1_clk_input*]
set_false_path -from [get_clocks *clk_out1_clk_input*] -to [get_clocks *clk_hbm*]

set_false_path -from [get_pins DAGgenController_inst/DAGArrangement_1_inst/*size_stack_reg*/C]
set_false_path -from [get_pins DAGgenController_inst/DAGArrangement_2_inst/*size_stack_reg*/C]

create_pblock pblock_keccak256_core_inst0
add_cells_to_pblock [get_pblocks pblock_keccak256_core_inst0] [get_cells -quiet [list keccak256_core_inst0]]
resize_pblock [get_pblocks pblock_keccak256_core_inst0] -add {CLOCKREGION_X0Y6:CLOCKREGION_X7Y7}


##
#### could use this to reduce bistream file size
#set_property BITSTREAM.GENERAL.COMPRESS True [current_design]



#### use REFCLK1
#set_property PACKAGE_PIN BD23 [get_ports refclk_1_p]
#set_property IOSTANDARD LVDS [get_ports refclk_1_p]
#set_property DIFF_TERM_ADV TERM_100 [get_ports refclk_1_p]

#set_property PACKAGE_PIN BD24 [get_ports refclk_1_n]
#set_property IOSTANDARD LVDS [get_ports refclk_1_n]
#set_property DIFF_TERM_ADV TERM_100 [get_ports refclk_1_n]


#### use REFCLK3
#set_property PACKAGE_PIN F13 [get_ports refclk_3_p]
#set_property IOSTANDARD LVDS [get_ports refclk_3_p]
#set_property DIFF_TERM_ADV TERM_100 [get_ports refclk_3_p]

#set_property PACKAGE_PIN F12 [get_ports refclk_3_n]
#set_property IOSTANDARD LVDS [get_ports refclk_3_n]
#set_property DIFF_TERM_ADV TERM_100 [get_ports refclk_3_n]

#### use REFCLK4
#set_property PACKAGE_PIN BC32 [get_ports refclk_4_p]
#set_property IOSTANDARD LVDS [get_ports refclk_4_p]
#set_property DIFF_TERM_ADV TERM_100 [get_ports refclk_4_p]

#set_property PACKAGE_PIN BC33 [get_ports refclk_4_n]
#set_property IOSTANDARD LVDS [get_ports refclk_4_n]
#set_property DIFF_TERM_ADV TERM_100 [get_ports refclk_4_n]


#### I2C
#set_property PACKAGE_PIN AY27 [get_ports i2c_sda]
#set_property PACKAGE_PIN AY26 [get_ports i2c_scl]
#set_property IOSTANDARD LVCMOS18 [get_ports i2c_scl]
#set_property IOSTANDARD LVCMOS18 [get_ports i2c_sda]

#### RESETN: this resetn signal is controlled by external device
#set_property PACKAGE_PIN BE17 [get_ports resetn]
#set_property IOSTANDARD LVCMOS18 [get_ports resetn]

