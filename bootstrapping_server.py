from flask import Flask, request, jsonify
from rule_enforcement import translate_to_iptables, enforce_ip_table
import requests
import subprocess

app = Flask(__name__)

ALLOWED_IPS = {'192.168.1.100', '10.0.0.1', '13.38.251.115', '127.0.0.1', '192.168.1.145'}
mud_server_IP = "http://127.0.0.1:5000" # IP address of the MUD server. Must be changed to the actual IP address of the MUD server!!!

def is_valid_ip(ip_address):
    # check if the IP address is in the allowed set
    return ip_address in ALLOWED_IPS

#Function to tell server to retrieve a MUD file for a device
#Send a request to the MUD server with a device ID, also verifies the MUD file
def search_mud_file(device_id, mud_server_IP, public_key):
    url = f"{mud_server_IP}/mud/{device_id}"
    try:
        response = requests.get(url)
        if response.status_code == 200:
            data = response.json()
            mud = data['mud']
            sig = data['sig']
            if not verify_mud_file(mud, sig, public_key):
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
    if not is_valid_ip(request.remote_addr):
        return jsonify({'error': 'Unauthorized IP address'}), 403
    public_key = requests.get(f"{mud_server_IP}/pk").json()['public_key']
    search_mud_file(device_id, mud_server_IP, public_key)
    return jsonify({'message': 'MUD file retrieval request sent to MUD server'}), 200

#Function to parse the MUD file
def parse_mud(mud):
    policies = []
    json_string = mud.decode('utf-8')
    import json
    json_data = json.loads(json_string)

    for direction in ['inbound', 'outbound']:
        for rule in json_data['mud']['policy']['acl'][direction]:
            policy = {
                'direction': direction,
                'protocol': rule['protocol'],
                'ports': rule['src-port'],
                'action': rule['action']
            }
            policies.append(policy)
    return policies

#Function to verify the MUD file's signature
#TODO test this function
def verify_mud_file(mudfile, signature, public_key):
    with open("temp_public_key.pem", "w") as f:
        f.write(public_key)
    with open("mudfile.sig", "wb") as f:
        f.write(signature)
    with open("temp_mudfile.json", "w") as f:
        f.write(mudfile)
    
    result = subprocess.run(
        ["openssl", "dgst", "-sha256", "-verify", "temp_public_key.pem", "-signature", "mudfile.sig", "temp_mudfile.json"],
        capture_output=True,
        check=True,
        text=True
    )
    
    subprocess.run(["rm", "temp_mudfile.json", "temp_public_key.pem", "mudfile.sig"]) 
    return "Verified OK" in result.stdout

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=6000)