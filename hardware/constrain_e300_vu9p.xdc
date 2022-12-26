##-----------------------------------------------------------------------------
##
## (c) Copyright 2012-2012 Xilinx, Inc. All rights reserved.
##
## This file contains confidential and proprietary information
## of Xilinx, Inc. and is protected under U.S. and
## international copyright and other intellectual property
## laws.
##
## DISCLAIMER
## This disclaimer is not a license and does not grant any
## rights to the materials distributed herewith. Except as
## otherwise provided in a valid license issued to you by
## Xilinx, and to the maximum extent permitted by applicable
## law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
## WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
## AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
## BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
## INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
## (2) Xilinx shall not be liable (whether in contract or tort,
## including negligence, or under any other theory of
## liability) for any loss or damage of any kind or nature
## related to, arising under or in connection with these
## materials, including for any direct, or any indirect,
## special, incidental, or consequential loss or damage
## (including loss of data, profits, goodwill, or any type of
## loss or damage suffered as a result of any action brought
## by a third party) even if such damage or loss was
## reasonably foreseeable or Xilinx had been advised of the
## possibility of the same.
##
## CRITICAL APPLICATIONS
## Xilinx products are not designed or intended to be fail-
## safe, or for use in any application requiring fail-safe
## performance, such as life-support or safety devices or
## systems, Class III medical devices, nuclear facilities,
## applications related to the deployment of airbags, or any
## other applications that could lead to death, personal
## injury, or severe property or environmental damage
## (individually and collectively, "Critical
## Applications"). Customer assumes the sole risk and
## liability of any use of Xilinx products in Critical
## Applications, subject only to applicable laws and
## regulations governing limitations on product liability.
##
## THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
## PART OF THIS FILE AT ALL TIMES.
##
##-----------------------------------------------------------------------------
##
## Project    : The Xilinx PCI Express DMA
## File       : xilinx_pcie_xdma_ref_board.xdc
## Version    : 4.1
##-----------------------------------------------------------------------------
#
# User Configuration
# Link Width   - x8
# Link Speed   - Gen2
# Family       - virtexuplusHBM
# Part         - xcu50
# Package      - fsvh2104
# Speed grade  - -2L
#
# PCIe Block INT - 7
# PCIe Block STR - PCIE4C_X1Y1
#
###############################################################################
# User Time Names / User Time Groups / Time Specs
###############################################################################
##
## Free Running Clock is Required for IBERT/DRP operations.
##
#############################################################################################################
#
#############################################################################################################
#set_property CONFIG_VOLTAGE 1.8 [current_design]
#

set_property IOSTANDARD LVCMOS18 [get_ports rx]
set_property PACKAGE_PIN AR15 [get_ports rx]
set_property IOSTANDARD LVCMOS18 [get_ports tx]
set_property PACKAGE_PIN AP14 [get_ports tx]

set_property PACKAGE_PIN AY25 [get_ports clk1_p]
set_property PACKAGE_PIN AY26 [get_ports clk1_n]
set_property PACKAGE_PIN AW20 [get_ports clk2_p]
set_property PACKAGE_PIN AW19 [get_ports clk2_n]
set_property PACKAGE_PIN BA14 [get_ports clk3_p]
set_property PACKAGE_PIN BA13 [get_ports clk3_n]
set_property PACKAGE_PIN AY18 [get_ports clk4_p]
set_property PACKAGE_PIN AY17 [get_ports clk4_n]

set_property IOSTANDARD LVDS [get_ports clk1_p]
set_property IOSTANDARD LVDS [get_ports clk1_n]
set_property IOSTANDARD LVDS [get_ports clk2_p]
set_property IOSTANDARD LVDS [get_ports clk2_n]
set_property IOSTANDARD LVDS [get_ports clk3_p]
set_property IOSTANDARD LVDS [get_ports clk3_n]
set_property IOSTANDARD LVDS [get_ports clk4_p]
set_property IOSTANDARD LVDS [get_ports clk4_n]

set_property DIFF_TERM TRUE  [get_ports clk1_p]
set_property DIFF_TERM TRUE  [get_ports clk2_p]
set_property DIFF_TERM TRUE  [get_ports clk3_p]
set_property DIFF_TERM TRUE  [get_ports clk4_p]

create_clock -period 10 -name clk1_p [get_ports clk1_p]
create_clock -period 10 -name clk2_p [get_ports clk2_p]
create_clock -period 10 -name clk3_p [get_ports clk3_p]
create_clock -period 10 -name clk4_p [get_ports clk4_p]

set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]

