from flask import Flask, jsonify, request
from key_generator import generate_keys
import subprocess

app = Flask(__name__)

ALLOWED_IPS = {'192.168.1.100', '10.0.0.1', '13.38.251.115', '127.0.0.1', '192.168.1.145'}

#Class to store MUD files in the inventory
class Inventory:
    def __init__(self):
        self.devices = {}
        self.device_policies = {}

    def store_mud(self, device_id, mud):
        self.devices[device_id] = mud

    def get_mud(self, device_id):
        return self.devices.get(device_id)

inventory = Inventory()

#TODO make reasonably safe
key_pair = generate_keys()

#Sample device MUD file in JSON format. This MUD file (if enforced correctly) would allow the IoT device to accept inbound HTTP and HTTPS
device_mud = {
        'name': 'Sample MUD Profile',
        'manufacturer': 'Example',
        'model': 'ABC123',
        'cache-validity': 48,
        'mud_url': "http://example.com/mud-files/ABC123",
        'last_update': "2019-04-17T09:47:00+00:00",
        'policy': {
            'acl': {
                'inbound': [
                    {
                        'name': 'allow_http',
                        'protocol': 'tcp',
                        'src-port': [80, 443],
                        'action': 'allow'
                    }
                ],
                'outbound': [
                    {
                        'name': 'allow_http',
                        'protocol': 'tcp',
                        'src-port': [80, 443],
                        'action': 'allow'
                    }
                ]
            }
        }
    }

#Function to check if the IP address is in the allowed set
def is_valid_ip(ip_address):
    # check if the IP address is in the allowed set
    return ip_address in ALLOWED_IPS

#Endpoint to retrieve MUD file for a specific device
#If the MUD file is not found in the inventory, the server just use the sample device MUD file for now
@app.route('/mud/<device_id>', methods=['GET'])
def retrieve_mud(device_id):
    mud = inventory.get_mud(device_id)
    if mud:
        print(f"MUD file found for device ID {device_id}")
        mud_sig = sign_mudfile(mud, key_pair[0])
        return jsonify({'mud': mud, 'sig': mud_sig}), 200
    else:
        inventory.store_mud(device_id, device_mud)
        print(f"sample MUD file used for device ID {device_id}")
        mud_sig = sign_mudfile(device_mud, key_pair[0])
        return jsonify({'mud': device_mud, 'sig': mud_sig}), 200
    
#Endpoint to add a MUD file to the inventory from outside the server   
@app.route('/mud/<device_id>', methods=['POST'])
def post_mud(device_id):
    new_mud = request.get_json()

    inventory.store_mud(device_id, new_mud)

    response = {
        'status': 'success',
        'message': 'Mud file added to inventory',
        'data': new_mud
    }

    return jsonify(response), 200

#Endpoint to retrieve the public key of the server
@app.route('/pk', methods=['GET'])
def retrieve_public_key():
    pk = key_pair[1]
    if pk:
        print(f"found public key {pk}")
        return jsonify({'public_key': pk}), 200
    else:
        print(f"no public key found")
        return jsonify({'error': 'public key not found'}), 404

#Function to sign the MUD file
#TODO test this function
def sign_mudfile(mudfile_json, private_key):
    with open("temp_private_key.pem", "w") as f:
        f.write(private_key)
    with open("temp_mudfile.json", "w") as f:
        f.write(mudfile_json)
    
    subprocess.run(
        ["openssl", "dgst", "-sha256", "-sign", "temp_private_key.pem", "-out", "mudfile.sig", "temp_mudfile.json"],
        check=True
    )
    
    with open("mudfile.sig", "rb") as f:
        signature = f.read()

    subprocess.run(["rm", "temp_mudfile.json", "temp_private_key.pem", "mudfile.sig"])
    return signature

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)