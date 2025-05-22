# artworkTrackingMAP

# Introduction
This README explains how to run the code in this repository in order to imitate the results of our thesis.
The code to get the servers working lies within the "artworkTrackingMAP" folder. The .bin file for the ST-board can be found in "artworkTrackingMAP\DEMO-CERTIFY-BOOTSTRAPPING-v3(1)\DEMO-CERTIFY-BOOTSTRAPPING-v3\Projects\B_L462E\Demonstrations\Cellular\IDE\STM32CubeIDE\Debug\Prj_Murata_L462.bin".

# Servers
## What Type
We used AWS EC2 ubuntu servers for the server structure. You can also run it locally. Make sure its a linux system though, as it will need to manipulate iptables to enforce MUD rules. If you run AWS EC2 instances make sure you whitelist the servers respectively. Also you must whitelist port 4000 for all communication on the server running the bootstrapping_server.py code. 

## How to get AWS EC2 ubuntu instances running

Connecting to an AWS EC2 instance via CMD:
*ssh -i path/to/key ubuntu@ip.address*

Uploading a file to AWS EC2 instance via CMD:
*scp -i path/to/key path/to/file ubuntu@ip.address:path/on/server*

Installing a Library on AWS Server:
*sudo apt install python3-nameOfLibrary*

You need two servers. On the first "Bootstrapping Server", you will need to have:
* bootstrapping_server.py
* Device.py
* key_generator.py
* rule_enforcement.py
* initial_ip_table_config.json

On the second "MUD Server", you will need to have:
* mud_server.py
* key_generator.py

Once the files are loaded (no folders needed, put every file at the same place), the dependencies must be added to the servers. 
