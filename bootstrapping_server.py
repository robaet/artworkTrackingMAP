from flask import Flask, request, jsonify
import requests

app = Flask(__name__)

ALLOWED_IPS = {'192.168.1.100', '10.0.0.1', '13.38.251.115', '127.0.0.1'}
mud_server_IP = "127.0.0.1:5000" # IP address of the MUD server. Must be changed to the actual IP address of the MUD server!!!

def is_valid_ip(ip_address):
    # check if the IP address is in the allowed set
    return ip_address in ALLOWED_IPS

#Function to tell MUD server to retrieve a MUD file for a device
#Send a request to the MUD server with a device ID and MUD file URL
def search_mud_file(mud_file_url, device_id, mud_server_IP):
    url = f"{mud_server_IP}/mud/{device_id}/{mud_file_url}"
    
    try:
        response = requests.get(url)
        if response.status_code == 200:
            pass
        else:
            print(f"Failed to request MUD file retrieval for device ID {device_id}. HTTP status code: {response.status_code}")
    except requests.RequestException as e:
        print(f"An error occurred in bootstrappig server trying to get MUD File for device ID {device_id}: {e}")

#Endpoint to receive request from IOT device to retrieve MUD file
@app.route('/mud/<device_id>/<mud_file_url>', methods=['GET'])
def retrieve_mud_file(device_id, mud_file_url):
    if not is_valid_ip(request.remote_addr):
        return jsonify({'error': 'Unauthorized IP address'}), 403
    search_mud_file(mud_file_url, device_id, mud_server_IP)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=6000)