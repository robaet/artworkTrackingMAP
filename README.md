# artworkTrackingMAP

# Introduction
This README explains how to run the code in this repository in order to imitate the results of our thesis.
The code to get the servers working lies within the "artworkTrackingMAP" folder. The .bin file for the ST-board can be found [here](DEMO-CERTIFY-BOOTSTRAPPING-v3(1)/DEMO-CERTIFY-BOOTSTRAPPING-v3/Projects/B_L462E/Demonstrations/Cellular/IDE/STM32CubeIDE/Debug/Prj_Murata_L462.bin).

# Servers
## What Type
We used AWS EC2 ubuntu servers for the server structure. You can also run it locally. Make sure its a linux system though, as it will need to manipulate iptables to enforce MUD rules. If you run AWS EC2 instances make sure you whitelist the servers respectively. Also you must whitelist port 4000 for all communication on the server running the bootstrapping_server.py code. 

## Getting Servers Ready

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

Once the files are loaded (no folders needed, put every file at the same place), the dependencies must be added to the servers. Make sure you upload all libraries to the servers you dont already have.

## Running Servers
To run the servers simply run the command "sudo python3 bootstrapping_server.py" on the Bootstrapping Server and "sudo python3 mud_server.py" on the MUD Server. The servers should now be functional and listen on various Ports. Once the servers are functional you can continue with the board.


# Board
As stated in the Thesis, we used the B-L462E-CELL1 IoT Discovery Kit from STMicroelectronics, with additional SIM slot and antenna. 
We used [STM32CUBEIDE](https://www.st.com/content/st_com/en/stm32cubeide.html) to edit and build the code of the board. We used version 1.14.1.
Open [the project file](DEMO-CERTIFY-BOOTSTRAPPING-v3(1)/DEMO-CERTIFY-BOOTSTRAPPING-v3/Projects/B_L462E/Demonstrations/Cellular/IDE/STM32CubeIDE) in the IDE.

Once the AWS server instances are running you need to adjust the MUD URL and IP from the Bootstrapping Server in [custom_client.c](DEMO-CERTIFY-BOOTSTRAPPING-v3(1)/DEMO-CERTIFY-BOOTSTRAPPING-v3/Middlewares/ST/STM32_Cellular/Samples/Custom/Src/custom_client.c).
* Replace the current value of `SERVER_LOG_IP` with the decimal value of the public IP of the AWS instance on which the Bootstrapping Server runs. You can use [this website](https://tools.iplocation.net/ip-to-integer-converter) for the IP conversion for example.
* Replace the IP in the current value of `MUD_URL` with the public IP of the AWS instance on which the Mud Server runs.

If u want to you can change how often the connectivity data is updated on the board's side in [plf_sw_config.h](DEMO-CERTIFY-BOOTSTRAPPING-v3(1)/DEMO-CERTIFY-BOOTSTRAPPING-v3/Projects/B_L462E/Demonstrations/Cellular/STM32_Cellular/App/plf_sw_config.h). Just change the value of the variable `CST_MODEM_POLLING_PERIOD` on line 55. However keep in mind that this does not influence the frequency of sent connectivity data to the Bootstrapping Server but only its actuality.

Don't forget to build the project after making changes to the source code of the board. You can do so via the top menu of the STM32CubeIDE: via "Project" -> "Build Project".

To run the board code you connect the board to your computer and open TeraTerm. In TeraTerm click on "Serial" then "ok". Then in TeraTerm click "setup", "Serial port" and set Speed to 115200 and press "new setting". Then send the binary file "Prj_Murata_L462.bin" to the board and enter "mems period 1000" into the command line.

# Data
If everything runs, the system will automatically log data on the Bootstrapping Server in the sensor_data.txt file. The connection_log.txt file logs from what IP connections have been established, if that is of interest.
In the /data folder you can find the data of the two test runs that was used in the Thesis.

## GPS
The GPS data was collected via Android App. All Android Studio Project files are within the LocationLogger folder. 
artworkTrackingMAP\LocationLogger\app\src\main\java\com\example\locationlogger\MainActivity.kt is the main file.
