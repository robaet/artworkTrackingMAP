from flask import Flask, request, jsonify
from rule_enforcement import translate_to_iptables, enforce_ip_table
import requests
import json
from another_key_generator import verify_signature
from apscheduler.schedulers.background import BackgroundScheduler
from datetime import datetime, timedelta
import atexit
import socket
import time
import threading
import signal
import subprocess

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

def get_current_iptables():
    try:
        result = subprocess.run(['iptables', '-L', '-n', '-v'], capture_output=True, text=True, check=True)
        return result.stdout
    except subprocess.CalledProcessError as e:
        return f"Error running iptables: {e.stderr}"
    except FileNotFoundError:
        return "iptables command not found. Are you sure it's installed on your system?"

def startTcpServer():
    print("Starting data handler port...")
    HOST = '0.0.0.0'
    PORT = 5000
    
    with open("temp_mudfile.json", "r") as f:
        mud = json.load(f)
    print(mud)
    #enforce_ip_table(translate_to_iptables(parse_mud(mud)))

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        server_socket.bind((HOST, PORT))
        server_socket.listen(5) # 5 connections
        server_socket.settimeout(1.0)  # Timeout after 1 second

        print(f"Server listening on {HOST}:{PORT}")

        while True:
            try:
                conn, addr = server_socket.accept()
            except socket.timeout:
                continue 
            with conn:
                print('Connected by', addr)
                buffer = b"" 
                while True:
                    data = conn.recv(1024)
                    if not data:
                        break
                    buffer += data
                    try:
                        messages = buffer.decode('utf-8').split('}')
                        print(messages)
                        # Re-add the closing bracket and process each JSON object
                        for msg in messages[:-1]:
                            if msg.strip():  # Ignore empty fragments
                                process_json_message(msg + '}')
                        # Keep the incomplete part for the next round
                        buffer = messages[-1].encode('utf-8')
                    except UnicodeDecodeError:
                        print("Received non-decodable bytes. Skipping.")
                        buffer = b""

def process_json_message(message):
    data_points = json.loads(message)
    print(f"Parsed JSON buffer: {data_points}")
    data_points = sorted(data_points, key=lambda x: int(x['time']))
    latest_time = int(data_points[-1]['time'])
    current_rtc = datetime.now()
    for point in data_points:
        delta_ms = latest_time - int(point['time'])
        point_rtc = current_rtc - timedelta(milliseconds=delta_ms)
        point['rtc_timestamp'] = point_rtc.strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]

    with open("sensor_data.txt", 'a') as file:
        for point in data_points:
            file.write(json.dumps(point) + '\n')
            print("Data written:", point)


def startHttpServer():
    app.run(host='0.0.0.0', port=6000)

def serverShutdown(sig, frame):
    print("Manual shutdown...")
    exit(0)

if __name__ == '__main__':
    print("Starting bootstrapping server...")
    scheduler = BackgroundScheduler()
    job = scheduler.add_job(updatePolicies, 'interval', seconds=10)
    scheduler.start()
    atexit.register(lambda: scheduler.shutdown())

    signal.signal(signal.SIGINT, serverShutdown)
    tcp_thread = threading.Thread(target=startTcpServer)
    flask_thread = threading.Thread(target=startHttpServer)
    tcp_thread.daemon = True
    flask_thread.daemon = True
    tcp_thread.start()
    flask_thread.start()
    tcp_thread.join()
    flask_thread.join()
    while True:
        time.sleep(2)