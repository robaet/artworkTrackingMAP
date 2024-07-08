from flask import Flask, request, jsonify
from rule_enforcement import translate_to_iptables, enforce_ip_table
import requests
import json
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.backends import default_backend
from another_key_generator import verify_signature

app = Flask(__name__)

ALLOWED_IPS = {'192.168.1.100', '10.0.0.1', '13.38.251.115', '127.0.0.1', '192.168.1.145', '192.168.1.120', '188.61.197.109'}
mud_server_IP = "http://127.0.0.1:5000" # IP address of the MUD server. Must be changed to the actual IP address of the MUD server!!!

def is_valid_ip(ip_address):
    # check if the IP address is in the allowed set
    return ip_address in ALLOWED_IPS

#Function to tell server to retrieve a MUD file for a device
#Send a request to the MUD server with a device ID, also verifies the MUD file
def search_mud_file(device_id, mud_server_IP, certificate):
    url = f"{mud_server_IP}/mud/{device_id}"
    try:
        response = requests.get(url)
        if response.status_code == 200:
            data = json.loads(response.content.decode('utf-8'))
            mud = data["mud"]
            sig = data["sig"].encode('utf-8')
            if not verify_signature(mud, sig, certificate):
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
    response = requests.get(f"{mud_server_IP}/certificate")
    certificate = 'downloaded_certificate.pem'
    with open(certificate, 'wb') as f:
        f.write(response.content)
        print(f"Certificate saved to {certificate}")
    search_mud_file(device_id, mud_server_IP, certificate)
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

#Function to verify the MUD file's signature
#TODO test this function
def verify_mud_file(mud, signature, key):
    print(type(mud))
    print(type(signature))
    print(type(key.content))

    signature_bytes = bytes.fromhex(signature)
    public_key_bytes = key.content


    # Load public key
    public_key = serialization.load_pem_public_key(public_key_bytes, backend=default_backend())

    # Convert the mudfile_dict to bytes
    mudfile_bytes = json.dumps(mud).encode('utf-8')

    print(signature_bytes)

    # Perform the verification
    try:
        verify_signature()
        print("Signature is valid.")
        return True
    except Exception as e:
        print(f"Verification failed: {str(e)}")
        return False

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=6000)