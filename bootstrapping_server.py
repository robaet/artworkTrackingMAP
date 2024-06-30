from flask import Flask, request, jsonify
import json
import requests

app = Flask(__name__)

ALLOWED_IPS = {'192.168.1.100', '10.0.0.1', '13.38.251.115', '127.0.0.1'}
mud_server_IP = "127.0.0.1:5000"

def is_valid_ip(ip_address):
    # check if the IP address is in the allowed set
    return ip_address in ALLOWED_IPS

def get_mud_file(mud_file_url, device_id, mud_server_IP):
    url = f"{mud_server_IP}/mud/{device_id}"
    
    try:
        response = requests.get(url)
        if response.status_code == 200:
            pass
        else:
            print(f"Failed to request MUD file retrieval for device ID {device_id}. HTTP status code: {response.status_code}")
    except requests.RequestException as e:
        print(f"An error occurred in bootstrappig server trying to get MUD File for device ID {device_id}: {e}")

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=6000)