from flask import Flask, jsonify, request, make_response
from key_generator import generate_key_pair, sign_file
import json
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives.asymmetric import utils
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.serialization import load_pem_private_key
from OpenSSL import crypto
import signal

app = Flask(__name__)

ALLOWED_IPS = {'192.168.1.100', '10.0.0.1', '13.38.251.115', '127.0.0.1', '192.168.1.145', '192.168.1.120', '213.55.241.102'}

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
private_key, certificate = generate_key_pair()

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
                'input': [
                    {
                        'name': 'allow_http',
                        'protocol': 'tcp',
                        'src-port': [80, 443],
                        'action': 'allow'
                    }
                ],
                'output': [
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
#todo maybe die spezifisch IP vom board dri tue
device_mud = {
"name": "Sample MUD Profile", "manufacturer": "Example", "model": "ABC123", "cache-validity": 48, "mud_url": "http://example.com/mud-files/ABC123", "last_update": "2019-04-17T09:47:00+00:00", 
 "policy": 
    {
        "acl": 
            {
                "input": [
                    {"name": "allow_tcp_data", "protocol": "tcp", "dst-port": [5000], "action": "ACCEPT"},
                ]
            }
    }
}
inventory.store_mud(1, device_mud)


#Function to check if the IP address is in the allowed set
def is_valid_ip(ip_address):
    # check if the IP address is in the allowed set
    return ip_address in ALLOWED_IPS

#Endpoint to retrieve MUD file for a specific device
#If the MUD file is not found in the inventory, the server just use the sample device MUD file for now
@app.route('/mud/<device_id>', methods=['GET'])
def get_mudfile(device_id):
    mud = inventory.get_mud(device_id)
    if mud:
        print(f"MUD file found for device ID {device_id}")
        mud2 = json.loads(json.dumps(mud, sort_keys=True))
        signature = sign_file(json.dumps(mud2).encode('utf-8'), private_key)

        return {"mud": mud2, "sig": signature.hex()}, 200
    else:
        inventory.store_mud(device_id, device_mud)
        print(f"sample MUD file used for device ID {device_id}")
        mud2 = json.loads(json.dumps(device_mud, sort_keys=True))
        signature = sign_file(json.dumps(mud2).encode('utf-8'), private_key)
        return {"mud": mud2, "sig": signature.hex()}, 200

#Endpoint to retrieve the public key of the server
@app.route('/certificate', methods=['GET'])
def retrieve_certificate():
    pk = certificate
    print(f"certificate: {pk}")
    if pk:
        print(f"found certificate {pk}")
        cert_pem = crypto.dump_certificate(crypto.FILETYPE_PEM, certificate).decode('utf-8')
        response = make_response(cert_pem)
        response.headers['Content-Type'] = '"application/pkcs7-signature"'
        device_id = "123"  # TODO: make this dynamic
        response.headers['mudfile_url'] = f'/mud/{device_id}'
        response.headers['device_id'] = device_id
        return response, 200
    else:
        print(f"no certificate found")
        return jsonify({'error': 'certificate not found'}), 404

def serverShutdown(sig, frame):
    print("Manual shutdown...")
    exit(0)
    
if __name__ == '__main__':
    print(private_key)
    print(certificate)
    app.run(host='0.0.0.0', port=6000)

    signal.signal(signal.SIGINT, serverShutdown)
