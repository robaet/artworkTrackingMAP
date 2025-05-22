import pandas as pd
from datetime import datetime, timedelta

sensor_df = pd.read_json('sensor_data.txt', lines=True)
sensor_df['rtc_timestamp'] = pd.to_datetime(sensor_df['rtc_timestamp'])
location_df = pd.read_csv('location_log.txt', header=None, names=['unix_timestamp', 'latitude', 'longitude'])
location_df['datetime'] = pd.to_datetime(location_df['unix_timestamp'], unit='s')

def find_closest_location(sensor_time, location_times):
    deltas = (location_times - sensor_time).abs()
    min_delta = deltas.min()
    if min_delta <= timedelta(seconds=10):
        return deltas.idxmin()
    return None

matched_data = []
for _, row in sensor_df.iterrows():
    match_idx = find_closest_location(row['rtc_timestamp'], location_df['datetime'])
    if match_idx is not None:
        loc = location_df.loc[match_idx]
        matched_row = row.to_dict()
        matched_row['latitude'] = loc['latitude']
        matched_row['longitude'] = loc['longitude']
        matched_data.append(matched_row)

matched_df = pd.DataFrame(matched_data)
matched_df.to_csv('merged_sensor_location.csv', index=False)