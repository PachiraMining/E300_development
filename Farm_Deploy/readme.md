# How can use this script
Step 1: download  all files ( changeVoltage.sh  ip_list.txt    runAlephium.sh  runRadiant.sh  stopMining.sh) and put in the same folder

Step 2: set  permission for those file: <pre> sudo chmod 777 *.sh </pre>

Step 3: Update your E300 ip address into ip_list.txt

- If you want to set voltage for all Boxes in ip_list.txt : change vccInt, vccHBM at changeVoltage.sh then run: <pre>  sudo ./changeVoltage.sh </pre>

- If you want to mine Alephium for all Boxes in in_list.txt: change Wallet, Pool, Clock at runAlephium.sh then run: <pre>  sudo ./runAlephium.sh  </pre>

- If you want to mine Radiant for all Boxes in in_list.txt: change Wallet, Pool, Clock at  runRadiant.sh then run:<pre>   sudo ./runRadiant.sh </pre>

- If you don't want to stop mining: <pre> sudo ./stopMining.sh </pre>


Thanks for reading, FlyingPanda love you 10 tons!
