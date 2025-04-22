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

# Server details
HOST = '16.170.228.194'  # Localhost
PORT = 5000        # Make sure this matches your server's port

# Message to send
data = '{"yves":"funf", "time": "1000001","temperature": "20C","humidity": "17%","acceleration": "[2, 3, 4]"}'

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
