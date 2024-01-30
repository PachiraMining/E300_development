#!/bin/bash

vccInt=700
vccHBM=1000
# Check if the file does not exist
if [ ! -f "ip_list.txt" ]; then
    echo "File ip_list.txt does not exist."
    exit 1
fi

# API endpoint and new data
api_endpoint="/controller/setVoltage"
api_data="{\"voltage_vccint\":$vccInt,\"voltage_hbm\":$vccHBM,\"boardId\":3}"


# Read IP addresses from the file and perform API calls
while IFS= read -r ip; do
    api_url="http://${ip}:8200${api_endpoint}"
    curl_command="curl --header \"Content-Type: application/json\" --request POST --data '${api_data}' ${api_url}"

    echo "Performing API call to ${api_url}..."
    
    # Execute the API call
    eval ${curl_command}
    
    # Check the result of the curl command and provide feedback
    if [ $? -eq 0 ]; then
        echo "API call successful!"
    else
        echo "API call failed."
    fi

    echo "---------------------"

done < "ip_list.txt"

echo "Completion!"
