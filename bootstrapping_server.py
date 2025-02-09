from flask import Flask, request, jsonify
from rule_enforcement import translate_to_iptables, enforce_ip_table
import requests
import json
from another_key_generator import verify_signature
from apscheduler.schedulers.background import BackgroundScheduler
import atexit
import socket

app = Flask(__name__)
class Inventory:
    def __init__(self):
        self.devices = set()

    def set_devices(self, device_id):
        self.devices.add(device_id)

    def get_devices(self):
        return self.devices

inventory = Inventory()

ALLOWED_IPS = {'192.168.1.100', '10.0.0.1', '13.38.251.115', '127.0.0.1', '192.168.1.145', '192.168.1.120', '188.61.197.109', '213.55.241.102'}

def is_valid_ip(ip_address):
    # check if the IP address is in the allowed set
    # return ip_address in ALLOWED_IPS
    return True #For testing purposes, allow all IP addresses

#Function to tell server to retrieve a MUD file for a device
#Send a request to the MUD server with a device ID, also verifies the MUD file
def search_mud_file(device_id, mud_server_IP, certificate_path):
    url = f"{mud_server_IP}/mud/{device_id}"
    try:
        response = requests.get(url)
        if response.status_code == 200:
            data = json.loads(response.content.decode('utf-8'))
            mud = data["mud"]
            sig = bytes.fromhex(data["sig"])
            if not verify_signature(json.dumps(mud).encode('utf-8'), sig, certificate_path):
                print(f"MUD file retrieved for device ID {device_id} is invalid.")
            else:
                enforce_ip_table(translate_to_iptables(parse_mud(mud)))
        else:
            print(f"Failed to request MUD file retrieval for device ID {device_id}. HTTP status code: {response.status_code}")
    except requests.RequestException as e:
        print(f"An error occurred in bootstrappig server trying to get MUD File for device ID {device_id}: {e}")

#Endpoint to receive request from IOT device to retrieve MUD file
@app.route('/mud/<device_id>', methods=['GET'])
def retrieve_mud_file(device_id):
    mud_server_IP = request.args.get('mud_server_url')
    if not is_valid_ip(request.remote_addr):
        return jsonify({'error': 'Unauthorized IP address'}), 403
    response = requests.get(f"{mud_server_IP}/certificate")
    certificate_path = f'{device_id}_certificate.pem'
    with open(certificate_path, 'wb') as f:
        f.write(response.content)
        print(f"Certificate saved to {certificate_path}")
    search_mud_file(device_id, mud_server_IP, certificate_path)
    inventory.set_devices(device_id)
    return jsonify({'message': 'MUD file retrieval request sent to MUD server'}), 200

#Endpoint to receive sensor data from IOT device
@app.route('/data/<device_id>', methods=['POST'])
def receive_sensor_data(device_id):
    if not is_valid_ip(request.remote_addr):
        return jsonify({'error': 'Unauthorized IP address'}), 403
    data = request.json
    if data:
        with open('sensor_data.txt', 'a') as file:
            file.write(data + '\n')
    print(f"Received data from device ID {device_id}: {data}")
    return jsonify({'message': 'Data received'}), 200

#Simple test endpoint to receive data
@app.route('/', methods=['GET'])
def log_sensor_data1():
    print(f"Received GET request from Board")
    return "GET received successfully.", 200

#Simple test endpoint to receive data
@app.route('/', methods=['POST'])
def log_sensor_data2():
    print(f"Received POST request from Board")
    request.data = request.data.decode('utf-8')
    request.data = json.loads(request.data)
    print(f"Received data from Board: {request.data}")
    time_data = request.data.get('time')
    temperature_data = request.data.get('temperature')
    humidity_data = request.data.get('humidity')
    accel_data = request.data.get('acceleration')
    print(f"Received time data from Board: {time_data}")
    print(f"Received temperature data from Board: {temperature_data}")
    print(f"Received humidity data from Board: {humidity_data}")
    print(f"Received acceleration data from Board: {accel_data}")
    if time_data and temperature_data and humidity_data and accel_data:
        with open('sensor_data.txt', 'a') as file:
            file.write(time_data + '!' + temperature_data + '\n')
            file.write(time_data + '!' + humidity_data + '\n')
            file.write(time_data + '!' + accel_data + '\n')
            print(f"Wrote all data to sensor_data.txt")
        return "All data logged successfully.", 200
    else:
        return "No data provided.", 400

#Function to parse the MUD file
def parse_mud(mud):
    policies = []

    for direction in ['input', 'output']:
        for rule in mud['policy']['acl'][direction]:
            policy = {
                'direction': direction,
                'protocol': rule['protocol'],
                'ports': rule['src-port'],
                'action': rule['action']
            }
            policies.append(policy)
    return policies

def updatePolicies():    
    for device_id in inventory.get_devices():
        url = f"http://localhost:6000/mud/{device_id}" #todo actual bootstrapping server IP
        response = requests.get(url)
        if response.status_code == 200:
            print(f"Successfully updated policies for device ID {device_id}")
        else:
            print(f"Failed to update policies for device ID {device_id}. HTTP status code: {response.status_code}")

def dataHandlerSocket():
    print("Starting data handler server...")
    HOST = '0.0.0.0'
    PORT = 5000
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
            server_socket.bind((HOST, PORT))
            server_socket.listen(5) # 5 connections

            print(f"Server listening on {HOST}:{PORT}")

            while True:
                conn, addr = server_socket.accept()
                with conn:
                    print('Connected by', addr)
                    data = conn.recv(1024)
                    if not data:
                        break
                    print('Received', repr(data))
                    conn.sendall(data)
    except KeyboardInterrupt:
        print("\nServer stopped by user.")

if __name__ == '__main__':
    print("Starting bootstrapping server...")
    dataHandlerSocket()
    scheduler = BackgroundScheduler()
    job = scheduler.add_job(updatePolicies, 'interval', seconds=10)
    scheduler.start()
    atexit.register(lambda: scheduler.shutdown())
    app.run(host='0.0.0.0', port=6000)
