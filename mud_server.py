from flask import Flask, jsonify, request

app = Flask(__name__)

ALLOWED_IPS = {'192.168.1.100', '10.0.0.1', '13.38.251.115', '127.0.0.1', '192.168.1.145'}

#Class to store MUD file and policies
class Inventory:
    def __init__(self):
        self.devices = {}
        self.device_policies = {}

    def store_mud(self, device_id, mud):
        self.devices[device_id] = mud

    def get_mud(self, device_id):
        return self.devices.get(device_id)

inventory = Inventory()

# sample device MUD file in JSON format. This MUD file (if enforced correctly) would allow the IoT device to accept inbound HTTP and HTTPS
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


def is_valid_ip(ip_address):
    # check if the IP address is in the allowed set
    return ip_address in ALLOWED_IPS

#Endpoint to retrieve MUD file for a specific device
#If the MUD file is not found in the inventory, the server just use the sample device MUD file for now
@app.route('/mud/<device_id>', methods=['GET'])
def retrieve_mud(device_id):
    #TODO do something with the MUD file url. So far the mudfiles are retrieved via device ID
    mud_file_url = request.args.get('mud_file_url')
    mud = inventory.get_mud(device_id)
    if mud:
        print(f"MUD file found for device ID {device_id}")
        return jsonify({'mud': mud}), 200
    else:
        inventory.store_mud(device_id, device_mud)
        print(f"sample MUD file used for device ID {device_id}")
        return jsonify({'mud': device_mud}), 200
    
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

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)