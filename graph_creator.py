import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates

# Load merged data
df = pd.read_csv('merged_sensor_location.csv')
df['rtc_timestamp'] = pd.to_datetime(df['rtc_timestamp'])

# Plot 1: Signal Strength (dbm) over time
plt.figure(figsize=(12, 5))
plt.plot(df['rtc_timestamp'], df['dbm'], linestyle='-', label='Signal Strength (dBm)')
plt.title('Signal Strength Over Time')
plt.xlabel('Time')
plt.ylabel('Signal Strength (dBm)')
plt.grid(True)
plt.xticks(rotation=45)
plt.tight_layout()
plt.savefig('plot_dbm_over_time.png')
plt.show()

# Plot 2: Temperature and Humidity over time (dual axis)
fig, ax1 = plt.subplots(figsize=(12, 5))
ax1.set_xlabel('Time')
ax1.set_ylabel('Temperature (°C)', color='tab:red')
ax1.plot(df['rtc_timestamp'], df['temperature'], color='tab:red', label='Temperature')
ax1.tick_params(axis='y', labelcolor='tab:red')

ax2 = ax1.twinx()
ax2.set_ylabel('Humidity (%)', color='tab:blue')
ax2.plot(df['rtc_timestamp'], df['humidity'], color='tab:blue', label='Humidity')
ax2.tick_params(axis='y', labelcolor='tab:blue')

fig.autofmt_xdate()
plt.title('Temperature and Humidity Over Time')
plt.tight_layout()
plt.savefig('plot_temp_humidity_over_time.png')
plt.show()

# Plot 3: Signal Strength on Map (Latitude vs Longitude)
plt.figure(figsize=(8, 6))
sc = plt.scatter(df['longitude'], df['latitude'], c=df['dbm'], cmap='viridis', marker='o')
plt.colorbar(sc, label='Signal Strength (dBm)')
plt.xlabel('Longitude')
plt.ylabel('Latitude')
plt.title('Signal Strength by Location')
plt.grid(True)
plt.tight_layout()
plt.savefig('plot_signal_strength_map.png')
plt.show()

# Plot 4: Temperature on Map (Latitude vs Longitude)
plt.figure(figsize=(8, 6))
sc = plt.scatter(df['longitude'], df['latitude'], c=df['temperature'], cmap='viridis', marker='o')
plt.colorbar(sc, label='Temperature (°C)')
plt.xlabel('Longitude')
plt.ylabel('Latitude')
plt.title('Temperature by Location')
plt.grid(True)
plt.tight_layout()
plt.savefig('plot_temp_map.png')
plt.show()

# Plot 5: Humidity on Map (Latitude vs Longitude)
plt.figure(figsize=(8, 6))
sc = plt.scatter(df['longitude'], df['latitude'], c=df['humidity'], cmap='viridis', marker='o')
plt.colorbar(sc, label='Humidity (%)')
plt.xlabel('Longitude')
plt.ylabel('Latitude')
plt.title('Humidity by Location')
plt.grid(True)
plt.tight_layout()
plt.savefig('plot_humidity_map.png')
plt.show()

# Plot 5: Signal Strength on Map (Latitude vs Longitude), with dbm = 0 excluded
filtered_df = df[df['dbm'] != 0]
plt.figure(figsize=(8, 6))
sc = plt.scatter(filtered_df['longitude'], filtered_df['latitude'], c=filtered_df['dbm'], cmap='viridis', marker='o')
plt.colorbar(sc, label='Signal Strength (dBm)')
plt.xlabel('Longitude')
plt.ylabel('Latitude')
plt.title('Signal Strength by Location (Excluding dbm = 0)')
plt.grid(True)
plt.tight_layout()
plt.savefig('plot_signal_strength_map_filtered.png')
plt.show()

# Plot 6: Signal Strength on Map (Latitude vs Longitude), transparent background
fig, ax = plt.subplots(figsize=(8, 6), facecolor='none')
sc = ax.scatter(
    filtered_df['longitude'],
    filtered_df['latitude'],
    c=filtered_df['dbm'],
    cmap='viridis',
    marker='o'
)
ax.set_facecolor('none')
ax.grid(False)
ax.set_xticks([])
ax.set_yticks([])
ax.set_xlabel('')
ax.set_ylabel('')
plt.savefig('signal_strength_overlay.png', transparent=True, dpi=300)
plt.show()