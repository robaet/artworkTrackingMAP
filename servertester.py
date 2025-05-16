'''
import requests

url = "http://127.0.0.1:5000"
# GET request
response = requests.get(url)
print("GET Response:", response.text, response.status_code)

# POST request
data = '{"time": "1000001","temperature": "20C","humidity": "17%","acceleration": "[2, 3, 4]"}'
response = requests.post(url, data)
print("POST Response:", response.text)
'''


import socket

import time
import threading
import signal
import json
from flask import request, jsonify, Flask
import requests

# Server details
HOST = '13.48.42.247'  # Localhost
PORT = 6000        # Make sure this matches your server's port

# Message to send
data = {
"name": "Sample MUD Profile", "manufacturer": "Example", "model": "ABC123", "cache-validity": 48, "mud_url": "http://example.com/mud-files/ABC123", "last_update": "2019-04-17T09:47:00+00:00", 
 "policy": 
    {
        "acl": 
            {
                "input": [
                    {"name": "allow_tcp_data", "protocol": "tcp", "dst-port": [5000], "action": "ACCEPT", },
                    {"name": "allow_tcp_data", "protocol": "tcp", "dst-port": [5000], "action": "ACCEPT", "src-ip":"195.65.5.77"},
                ]
            }
    }
}
#data = "http://13.48.42.247:6000/mud/234"

def ping_server():

    try:
        # Create a socket object (IPv4, TCP)
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((HOST, PORT))
            print(f"Connected to {HOST}:{PORT}")

            s.sendall(data.encode())  # Send encoded message
            print(f"Sent: {data}")

            # Optionally receive a response
            response = s.recv(1024)
            print(f"Received: {response.decode()}")

    except ConnectionRefusedError:
        print(f"Could not connect to the server at {HOST}:{PORT}")

def upload_mud_file(device_id):
    try:
        print("hallooooooo")
        headers = {'Content-Type': 'application/json'}
        response = requests.post(f'http://13.60.104.114:6000/mud/upload/{device_id}', data=json.dumps(data), headers=headers)
        response.raise_for_status()  # Raise an error for bad responses (4xx and 5xx)
        return response
    except requests.exceptions.RequestException as e:
        print(f"Error occurred: {e}")
        return None

def serverShutdown(sig, frame):
    print("Manual shutdown...")
    exit(0)

if __name__ == '__main__':
    upload_mud_file(1)
    #ping_server()
    signal.signal(signal.SIGINT, serverShutdown)