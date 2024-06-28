import matplotlib.pyplot as plt
import datetime
import time

while(True):
    # Read the Tera Term log file
    with open("teraterm_exp_0.log", "r") as log_data:
        lines = log_data.readlines()
    
    # Parse the log data
    timestamps, temperatures, humidities = [], [], []
    accelerations_x, accelerations_y, accelerations_z = [], [], []
    last_temperature, last_humidity, last_x, last_y, last_z = None, None, None, None, None
    for line in lines:  
        if ']  ' not in line:  # Skip lines without the expected format
            continue
        timestamp_str, data_str = line.split(']  ')
        timestamp = datetime.datetime.strptime(timestamp_str[1:], '%Y-%m-%d %H:%M:%S.%f')
        data_parts = data_str.split()
        if len(data_parts) < 3:
            continue  # Skip lines with insufficient data
        data_type, equal, value, unit = data_parts[:4]
        try:
            value = float(value)
        except ValueError:
            continue  # Skip lines where the value is not a valid float
        if data_type == 'TEMPERATURE':
            last_temperature = value
        elif data_type == 'HUMIDITY':
            last_humidity = value
        elif data_type == 'ACCELERATION':
            # Extract acceleration components
            data_type, equal, x_value, y_value, z_value = data_parts[:5]
            last_x = x_value
            last_y = y_value
            last_z = z_value
        if last_temperature is not None and last_humidity is not None and last_x is not None and last_y is not None and last_z is not None:
            timestamps.append(timestamp)
            temperatures.append(last_temperature)
            humidities.append(last_humidity)
            accelerations_x.append(last_x)
            accelerations_y.append(last_y)
            accelerations_z.append(last_z)
            last_temperature, last_humidity, last_x, last_y, last_z = None, None, None, None, None
            
    # Create separate plots for temperature and humidity
    plt.figure(figsize=(12, 8))
    
    # Plot for temperature
    plt.subplot(5, 1, 1)
    plt.plot(timestamps, temperatures, label='Temperature (°C)', color='red')
    plt.xlabel('Time')
    plt.ylabel('Temperature (°C)')
    plt.title('Temperature over Time')
    plt.legend()
    plt.grid(True)
    
    # Plot for humidity
    plt.subplot(5, 1, 2)
    plt.plot(timestamps, humidities, label='Humidity (%)', color='blue')
    plt.xlabel('Time')
    plt.ylabel('Humidity (%)')
    plt.title('Humidity over Time')
    plt.legend()
    plt.grid(True)
    
    # Plot for acceleration x
    plt.subplot(5, 1, 3)
    plt.plot(timestamps, accelerations_x, label='Acceleration X (mg)', color='green')
    plt.xlabel('Time')
    plt.ylabel('Acceleration (mg)')
    plt.title('Acceleration over Time X-Axis')
    plt.yticks([-100, -50, 0, 50, 100])
    plt.legend()
    plt.grid(True)
    
    # Plot for acceleration y
    plt.subplot(5, 1, 4)
    plt.plot(timestamps, accelerations_y, label='Acceleration Y (mg)', color='orange')
    plt.xlabel('Time')
    plt.ylabel('Acceleration (mg)')
    plt.title('Acceleration over Time Y-Axis')
    plt.yticks([-100, -50, 0, 50, 100])
    plt.legend()
    plt.grid(True)
    
    # Plot for acceleration z
    plt.subplot(5, 1, 5)
    plt.plot(timestamps, accelerations_z, label='Acceleration Z (mg)', color='purple')
    plt.xlabel('Time')
    plt.ylabel('Acceleration (mg)')
    plt.title('Acceleration over Time Z-Axis')
    plt.yticks([-100, -50, 0, 50, 100])
    plt.legend()
    plt.grid(True)

    
    plt.tight_layout()  # Adjust layout to prevent overlapping labels
    plt.show()
    
    time.sleep(5)