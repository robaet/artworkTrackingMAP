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
import re
import struct

class Inventory:
    def __init__(self):
        self.devices = set()

    def set_devices(self, device_id):
        self.devices.add(device_id)

    def get_devices(self):
        return self.devices

inventory = Inventory()

DATA_SERVER_PORT = 5000
MUD_LINK_SERVER_PORT = 4000
ERROR_PORT = 65535


#Function to tell server to retrieve a MUD file for a device
#Send a request to the MUD server with a device ID, also verifies the MUD file
def get_mudfile(mudfile_url, device_id, certificate_path):
    try:
        response = requests.get(mudfile_url)
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
        print(f"An error occurred in bootstrapping server trying to get MUD File for device ID {device_id}: {e}")

def get_public_key_mudfile(certificate_url):
    print("certificate url: " + certificate_url)
    try:
        response = requests.get(certificate_url)
            
        mudfile_url = response.headers.get('mudfile_url')
        mudfile_url = certificate_url.strip("/certificate")+mudfile_url
        print("mudfileurl: "+mudfile_url)

        device_id = response.headers.get('device_id')
        certificate_path = f'{device_id}_certificate.pem'
        with open(certificate_path, 'wb') as f:
            f.write(response.content)
            print(f"Certificate saved to {certificate_path}")
        get_mudfile(mudfile_url, device_id, certificate_path)
        inventory.set_devices(device_id)
    except requests.RequestException as e:
        print(f"An error occurred while retrieving the certificate: {e}")
        return ERROR_PORT
    return DATA_SERVER_PORT

#Function to parse the MUD file
def parse_mud(mud):
    policies = []
    acl = mud['policy']['acl']

    for direction, rules in acl.items():
        for rule in rules:
            policy = {
                'direction': direction,
                'protocol': rule['protocol'],
                'src-ports': rule.get('src-port', []),
                'dst-ports': rule.get('dst-port', []),  # Use get() in case dst-port is missing
                'action': rule['action']
            }

            # Optional IP-based filtering
            if 'src-ip' in rule:
                policy['src-ip'] = rule['src-ip']
            if 'dst-ip' in rule:
                policy['dst-ip'] = rule['dst-ip']

            policies.append(policy)
    return policies


def delete_all_rules():
    try:
        subprocess.run(['iptables', '-F'], check=True)
        print("All iptables rules deleted.")
    except subprocess.CalledProcessError as e:
        print(f"Error deleting iptables rules: {e}")

# TODO
'''
def update_policies():    
    for device_id in inventory.get_devices():
        url = f"http://{localhost}:{6000}/mud/{device_id}" #todo actual bootstrapping server IP
        response = requests.get(url)
        if response.status_code == 200:
            print(f"Successfully updated policies for device ID {device_id}")
        else:
            print(f"Failed to update policies for device ID {device_id}. HTTP status code: {response.status_code}")'''

def get_current_iptables():
    try:
        result = subprocess.run(['iptables', '-L', '-n', '-v'], capture_output=True, text=True, check=True)
        return result.stdout
    except subprocess.CalledProcessError as e:
        return f"Error running iptables: {e.stderr}"
    except FileNotFoundError:
        return "iptables command not found. Are you sure it's installed on your system?"

def start_tcp_data_server():
    print("Starting data handler port...")
    HOST = '0.0.0.0'
    PORT = DATA_SERVER_PORT
    delete_all_rules()
    print("#################       iptable rules BEFORE update       ###########\n\n" + get_current_iptables())
    with open("initial_ip_table_config.json", "r") as f:
        initial_ip_table_config = json.load(f)
    enforce_ip_table(translate_to_iptables(parse_mud(initial_ip_table_config)))
    print("#################       iptable rules AFTER update       ###########\n\n" + get_current_iptables())


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
                        messages = re.split(r'(?<=\})', buffer.decode('utf-8'))
                        print("messages: ")
                        print(messages)
                        buffer = messages[-1].encode('utf-8')
                    except UnicodeDecodeError:
                        print("Received non-decodable bytes. Skipping.")
                        buffer = b""

def start_tcp_mudlink_server():
    print("Starting TCP MUD link server...")
    HOST = '0.0.0.0'
    PORT = MUD_LINK_SERVER_PORT
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
                        messages = buffer.decode('utf-8')
                        print("messages 4000: ")
                        print(messages)
                        port = get_public_key_mudfile(messages)
                        buffer = messages[-1].encode('utf-8')

                        conn.sendall(struct.pack('!H', port))
                        print(f"Sent port {port} back to the sender.")
                    except UnicodeDecodeError:
                        print("Received non-decodable bytes. Skipping.")
                        buffer = b""
                        conn.sendall(struct.pack('!H', ERROR_PORT))

def process_json_message(message):
    data_points = []
    for msg in message:
        try:
            data_points.append(json.loads(msg))
        except json.JSONDecodeError:
            print("JSON decode error. Skipping message:", msg)
            continue
    print("Received message:", message)
    print("Received data:", data_points)
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



def serverShutdown(sig, frame):
    print("Manual shutdown...")
    exit(0)

if __name__ == '__main__':
    print("Starting bootstrapping server...")
    #scheduler = BackgroundScheduler()
    #job = scheduler.add_job(update_policies, 'interval', seconds=10)
    #scheduler.start()
    #atexit.register(lambda: scheduler.shutdown())

    signal.signal(signal.SIGINT, serverShutdown)
    tcp_data_thread = threading.Thread(target=start_tcp_data_server)
    tcp_mudlink_thread = threading.Thread(target=start_tcp_mudlink_server)
    tcp_data_thread.daemon = True
    tcp_mudlink_thread.daemon = True
    tcp_data_thread.start()
    tcp_mudlink_thread.start()
    tcp_data_thread.join()
    tcp_mudlink_thread.join()
    while True:
        time.sleep(2)