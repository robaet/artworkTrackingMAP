from flask import Flask, request, jsonify
from rule_enforcement import translate_to_iptables, enforce_ip_table, delete_all_rules, get_current_iptables, remove_rules_with_certain_ip
import requests
import json
from key_generator import verify_signature
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
from Device import Device
from urllib.parse import urlparse

class Inventory:
    def __init__(self):
        self.devices = set()

    def set_devices(self, device: Device):
        self.devices.add(device)

    def get_devices(self):
        return self.devices
    def remove_device(self, device: Device):
        print(f"Device with ID {device.getId()} removed from inventory.")
        self.devices.remove(device)

inventory = Inventory()

DATA_SERVER_PORT = 5000
MUD_LINK_SERVER_PORT = 4000
ERROR_PORT = 65535


#Function to tell server to retrieve a MUD file for a device
#Send a request to the MUD server with a device ID, also verifies the MUD file
def get_mudfile(mudfile_url, device_id, certificate_path, fromupdate):
    try:
        headers = {
            "Accept": "application/mud+json",
            "Accept-Language": "en",
            "User-Agent": "Bootstrapping_Server/1.0"
        }
        response = requests.get(mudfile_url, headers=headers)
        if response.status_code == 200:
            data = json.loads(response.content.decode('utf-8'))
            mud = data["mud"]
            sig = bytes.fromhex(data["sig"])
            if not verify_signature(json.dumps(mud).encode('utf-8'), sig, certificate_path):
                print(f"MUD file retrieved for device ID {device_id} is invalid.")
                return False
            else:
                enforce_ip_table(translate_to_iptables(parse_mud(mud)))
                return True
        else:
            print(f"Failed to request MUD file retrieval for device ID {device_id}. HTTP status code: {response.status_code}")
            return False
    except requests.RequestException as e:
        print(f"An error occurred in bootstrapping server trying to get MUD File for device ID {device_id} {fromupdate}: {e}")
        return False

def get_public_key_mudfile(certificate_url, addr):
    print("certificate url: " + certificate_url)
    try:
        headers = {
            "Accept": "application/mud+json",
            "Accept-Language": "en",
            "User-Agent": "Bootstrapping_Server/1.0"
        }
        response = requests.get(certificate_url, headers=headers)

        mudfile_url = response.headers.get('mudfile_url')
        parsed = urlparse(certificate_url)
        mudfile_url = f"{parsed.scheme}://{parsed.netloc}"+mudfile_url
        print("mudfileurl: "+mudfile_url)

        device_id = response.headers.get('device_id')
        certificate_path = f'{parsed.hostname}_{device_id}_certificate.pem'
        with open(certificate_path, 'wb') as f:
            f.write(response.content)
            print(f"Certificate saved to {certificate_path}")
        if not get_mudfile(mudfile_url, device_id, certificate_path, False):
            print(f"Failed to retrieve MUD file for device ID {device_id}.")
            return ERROR_PORT
        inventory.set_devices(Device(device_id, certificate_path, mudfile_url, addr))
    except Exception as e:
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

def update_policies():
    devices_to_remove = []
    for device in inventory.get_devices():
        print(f"update_policies: {device.getMudUrl()}, {device.getId()}, {device.getCertificate()}")
        
        if not get_mudfile(device.getMudUrl(), device.getId(), device.getCertificate(), True):
            print(f"Failed to retrieve MUD file for device ID {device.getId()}.")
            remove_rules_with_certain_ip(device.getIpAddress())
            devices_to_remove.append(device)
        else:
            print(f"Policies updated for device ID {device.getId()}")

    for deleted_device in devices_to_remove:
        inventory.remove_device(deleted_device)
    print("current policies: ")
    print(get_current_iptables())
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
                with open("connection_log.txt", "a") as log_file:
                    log_entry = f"{datetime.now().isoformat()} - Connected by {addr[0]}\n"
                    log_file.write(log_entry)
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
                        process_json_message(messages)
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
                        port = get_public_key_mudfile(messages, addr[0])
                        buffer = messages[-1].encode('utf-8')

                        conn.sendall(struct.pack('!H', port))
                        print(f"Sent port {port} back to the sender.")
                    except UnicodeDecodeError:
                        print("Received non-decodable bytes. Skipping.")
                        buffer = b""
                        conn.sendall(struct.pack('!H', ERROR_PORT))
                        print(f"Sent port {ERROR_PORT} back to the sender.")

def process_json_message(message):
    data_points = []
    for msg in message:
        try:
            if msg == "":
                continue
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
    scheduler = BackgroundScheduler()
    job = scheduler.add_job(update_policies, 'interval', seconds=10)
    scheduler.start()
    atexit.register(lambda: scheduler.shutdown())

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