#!/bin/bash

Pool="stratum%%2Btcp://pool.viporlab.net:16059" # %%2B is character +
Wallet="1ueouD2tTePMuXpkeTpXcahwgjGhDBMcWrR3ZS4zuNWh"
WorkerIDPrefix="E300" #WorkerID = WorkerIDPrefix_lastnumber of workerID
Clock="770"

# Check if the file doesn't exist
if [ ! -f "ip_list.txt" ]; then
    echo "File ip_list.txt does not exist."
    exit 1
fi

# API endpoint path and new data
api_endpoint="/cgi-bin/qcmap_web_cgi.cgi"
api_data_template="Page=setDracaenaMiner&status=0&info_a=wf&info_o=$Pool&info_u=$Wallet&info_miner=$WorkerIDPrefix%s&info_fpga_clk_core=$Clock&info_fpga_clk_core1=&info_fpga_clk_core2=&info_fpga_clk_mem=1100&info_fpga_clk_mem1=1100&info_fpga_clk_mem2=1100&auto_start=1&running_mode=0&input_in_oneline=+&input_in_oneline1=+&input_algo=alephium&token=1"

# Read IP addresses from the file and perform API calls
while IFS= read -r ip; do
    # Extract the number after the last dot in the IP address
    info_miner=$(echo "$ip" | awk -F'.' '{print $4}')

    # Create specific API data for each IP
    api_data=$(printf "$api_data_template" "$info_miner")

    api_url="http://${ip}${api_endpoint}"
    curl_command="curl -X POST -d '${api_data}' -H 'Content-Type: application/x-www-form-urlencoded' ${api_url}"

    echo "Performing API call to ${api_url}..."
    
    # Execute the API call
    eval "${curl_command}"
    
    # Check the result of the curl command and provide feedback
    if [ $? -eq 0 ]; then
        echo "API call successful!"
    else
        echo "API call failed."
    fi

    echo "---------------------"

done < "ip_list.txt"

echo "Completed!"
