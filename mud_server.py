from flask import Flask, request, jsonify
import json

app = Flask(__name__)

ALLOWED_IPS = {'192.168.1.100', '10.0.0.1', '13.38.251.115', '127.0.0.1'}

class Inventory:
    def __init__(self):
        self.devices = {}

    def store_mud(self, device_id, mud):
        self.devices[device_id] = mud

    def get_mud(self, device_id):
        return self.devices.get(device_id)

inventory = Inventory()


def is_valid_ip(ip_address):
    # check if the IP address is in the allowed set
    return ip_address in ALLOWED_IPS

@app.route('/mud', methods=['POST'])
def mud_request():
    data = request.get_json()

    if 'device_id' not in data:
        return jsonify({'error': 'Device ID not provided'}), 400

    device_id = data['device_id']

    # get client IP address from request
    client_ip = request.remote_addr
    print("MUD request from: ", client_ip)

    # validate client IP address
    if not is_valid_ip(client_ip):
        return jsonify({'error': 'Unauthorized IP address'}), 401

    # sample device MUD file in JSON format. This MUD file (if enforced correctly) would allow the IoT device to accept inbound HTTP and HTTPS
    device_mud = {
        'name': 'Sample MUD Profile',
        'manufacturer': 'Example',
        'model': 'ABC123',
        'policy': {
            'acl': {
                'inbound': [
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

    # currently not in use: threat MUD is a specific form of a MUD file which describes a particular cyber threat
    threat_mud = {
    "ietf-threatmud:mud": {
        "threat-mud-version": 1,
        "threat-mud-url": "https://ThreatMudServer/b0f62e766810b48fe19cc05a90e9997aae94df55b6ea9cef83ec4da0d3239116",
        "threat-id": "b0f62e766810b48fe19cc05a90e9997aae94df55b6ea9cef83ec4da0d3239116",
        "last-update": "2019-04-17T09:47:00+00:00",
        "cache-validity": 48,
        "is-supported": True,
        "Threat-Intelligence-Providers": [
            "AVAST\nFINDING: ELF:Filecoder-AF[Trj]"
        ],
        "cvss-vector": "¿?",
        "systeminfo": "Dangerous behaviour, seems DGA domain that can contain malware",
        "from-device-policy": {
            "access-lists": {
                "access-list": [
                ]
            },
            "mspl-list": {
                "mspls": [
                    {
                        "name": "mspl_42c0f156-6ee5-4e86-93d9-bfe568d267ae"
                    }
                ]
            }
        },
        "to-device-policy": {
            "access-lists": {
                "access-list": [
                ]
            },
            "mspl-list": {
                "mspls": [
                    {
                        "name": "mspl_42c0f156-6ee5-4e86-93d9-bfe568d267ae"
                    }
                ]
            }
        }
    },
    "ietf-access-control-list:acls": {
        "acl": []
    },
    "umu-mspl-list:mspls": {
        "mspl": [
            {
                "name": "mspl_42c0f156-6ee5-4e86-93d9-bfe568d267ae",
                "type": "ipv4-mspl-type",
                "configuration": {
                    "capability": "Filtering_L4",
                    "rule-set-configuration": {
                        "name": "mspl_set_653cb05e-1a92-4ce7-bafe-01f265436ff3",
                        "configuration-rule": [
                            {
                                "name": "Rule_d748f2a9-ed32-4b4d-bd87-a1cf5e9854f5",
                                "isCNF": False,
                                "external-data": {
                                    "priority": 500
                                },
                                "configuration-action": {
                                    "bootstrapping-action": {
                                        "bootstrapping-action-type": "UPDATE",
                                        "bootstrapping-model-version": "0.2",
                                        "network-bootstrapping-model": "((PortclSrc-42,UDP-45)=([1.0,1.0],[0.0,0.0]) OR (PortclSrc-42,UDP-45)=([0.0,0.0],[-1.0,-1.0]) OR (PortclSrc-42,UDP-45)=([2.0,2.0],[1.0,1.0])) AND ((PortclSrc-42,DHCP-45)=([1.0,1.0],[0.0,0.0]) OR (PortclSrc-42,DHCP-45)=([0.0,0.0],[-1.0,-1.0]) OR (PortclSrc-42,DHCP-45)=([2.0,2.0],[1.0,1.0]))",
                                        "network-bootstrapping-model-signature": "3ca5d818857d9f13efe6d78864f88a37d726d4c71"
                                    }
                                },
                                "configuration-condition": {
                                    "bootstrapping-configuration-condition": {
                                        "device-condition": {
                                            "name": "Aria"
                                        }
                                    }
                                }
                            }
                        ]
                    }
                }
            }
        ]
    }
    }

    # store MUD in inventory
    inventory.store_mud(device_id, device_mud)
    print("Stored MUD profile for device ID: ", device_id)

    return jsonify({'status': 'OK'}), 200

@app.route('/mud/<device_id>', methods=['GET'])
def get_mud(device_id):
    mud = inventory.get_mud(device_id)
    if mud:
        return jsonify({'mud': mud}), 200
    else:
        return jsonify({'error': 'MUD not found'}), 404

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)