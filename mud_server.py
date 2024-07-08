from flask import Flask, jsonify, request, make_response
from another_key_generator import generate_key_pair, sign_file
import json
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives.asymmetric import utils
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.serialization import load_pem_private_key
from OpenSSL import crypto

app = Flask(__name__)

ALLOWED_IPS = {'192.168.1.100', '10.0.0.1', '13.38.251.115', '127.0.0.1', '192.168.1.145', '192.168.1.120'}

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
        #mud_sig = sign_mudfile(mud, key_pair[0])
        signature = sign_file(mud, private_key)
        return {"mud": device_mud, "sig": signature.hex()}, 200
    else:
        inventory.store_mud(device_id, device_mud)
        print(f"sample MUD file used for device ID {device_id}")
        #mud_sig = sign_mudfile(device_mud, key_pair[0])
        mud = "test"
        signature = sign_file(mud, private_key)
        return {"mud": device_mud, "sig": signature.hex()}, 200
    
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
@app.route('/certificate', methods=['GET'])
def retrieve_certificate():
    print("here?")
    pk = certificate
    print("after?")
    if pk:
        print(f"found certificate {pk}")
        cert_pem = crypto.dump_certificate(crypto.FILETYPE_PEM, certificate).decode('utf-8')
        response = make_response(cert_pem)
        response.headers['Content-Type'] = '"application/pkcs7-signature"'
        return response, 200
    else:
        print(f"no certificate found")
        return jsonify({'error': 'certificate not found'}), 404

#Function to sign the MUD file
#TODO test this function
@DeprecationWarning
def sign_mudfile(mud, pk):
    # Load private key
    private_key = load_pem_private_key(pk, password=None, backend=default_backend())

    # Serialize JSON dictionary to bytes
    json_data = json.dumps(mud, separators=(',', ':')).encode('utf-8')

    # Calculate the digest of the data
    digest = hashes.Hash(hashes.SHA256(), backend=default_backend())
    digest.update(json_data)
    hashed_data = digest.finalize()

    # Sign the hashed data
    signature = private_key.sign(
        hashed_data,
        padding.PSS(
            mgf=padding.MGF1(hashes.SHA256()),
            salt_length=padding.PSS.MAX_LENGTH
        ),
        utils.Prehashed(hashes.SHA256())
    )

    return signature
    
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)