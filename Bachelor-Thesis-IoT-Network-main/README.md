# Secure IoT Network Prototyping for Artwork Tracking

## Steps to Use the Monitoring System

1. **Download Zip File**
   - Download the project zip file from [https://github.com/Islemmdimagh/Bachelor-Thesis-IoT-Network].

2. **Connect the Board (USB ST-LINK)**
   - Connect the board to your device using a USB micro cable.

3. **Launch Tera Term**
   - Launch Tera Term, select ”Serial” and choose ”COM3: STMicroelectronics STLink
     Virtual COM Port (COM3)” as the ”Port”.

4. **Adjust Baud Rate**
   - In Tera Term, under "Setup" > "Serial Port...", adjust the baud rate to 115200.

5. **Log Data with Timestamps**
   - In Tera Term, go to "File" > "Log..." to start logging data with timestamps.

6. **Run the Temperature Project**
   - Run the "Temperature" project on the connected board.

7. **Run TeraTermPlot.py**
   - Specify the log file name and location chosen in step 5.
   - After logging data, run TeraTermPlot.py.
