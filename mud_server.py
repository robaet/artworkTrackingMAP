from flask import Flask, request, jsonify
import json
import requests
import logging
import MudFileObject

app = Flask(__name__)

ALLOWED_IPS = {'192.168.1.100', '10.0.0.1', '13.38.251.115', '127.0.0.1', '192.168.1.145'}
mud_file_server_IP = "http://127.0.0.1:7000" # IP address of the MUD FIle server. Must be changed to the actual IP address of the MUD File server!!!

#Class to store MUD file and policies
class Inventory:
    def __init__(self):
        self.devices = {}
        self.device_policies = {}

    def store_mud(self, device_id, mud):
        self.devices[device_id] = mud

    def get_mud(self, device_id):
        return self.devices.get(device_id)
    
    def store_mud_policies(self, device_id, mud_object):
        self.device_policies[device_id] = mud_object

    def get_mud_policies(self, device_id):
        return self.device_policies.get(device_id)

inventory = Inventory()


def is_valid_ip(ip_address):
    # check if the IP address is in the allowed set
    return ip_address in ALLOWED_IPS

#Endpoint to retrieve MUD file for a specific device
#If the MUD file is not found in the inventory, the server will attempt to fetch it from the MUD File server
@app.route('/mud/<device_id>', methods=['GET'])
def retrieve_mud(device_id):
    mud_file_url = request.args.get('mud_file_url')
    mud = inventory.get_mud(device_id)
    if mud:
        return jsonify({'mud': mud}), 200
    else:
        get_mud_file(mud_file_url, device_id)
        if mud:
            return jsonify({'mud': mud}), 200
        else:
            return jsonify({'error': 'MUD not found'}), 404

#Function to convert JSON data to MudFileObject
def convert_json_to_object(mud_file):
        data = json.load(mud_file)
        return MudFileObject(
            name=data['name'],
            manufacturer=data['manufacturer'],
            model=data['model'],
            policy=data['policy'],
            last_update=data['last_update'],
            mud_url=data['mud_url'],
            cache_validity=data['cache-validity'],
            inbound=data['policy']['acl']['inbound'],
            outbound=data['policy']['acl']['outbound']
        )

#Function to parse MUD file and extract policies
def parse_mud(mud):
    policies = []
    for direction in ['inbound', 'outbound']:
        for rule in mud['policy']['acl'][direction]:
            policy = {
                'direction': direction,
                'protocol': rule['protocol'],
                'ports': rule['src-port'],
                'action': rule['action']
            }
            policies.append(policy)
    return policies

#Function to convert policies to iptables rules
def translate_to_iptables(policies):
    iptables_rules = []
    for policy in policies:
        for port in policy['ports']:
            rule = f"iptables -A {policy['direction'].upper()} -p {policy['protocol']} --dport {port} -j " + "ACCEPT" if {policy['action'].upper()=="allow"} else "DROP"
            iptables_rules.append(rule)
    return iptables_rules

#Function to fetch MUD file from the MUD File server
#todo periodically check for updates to the MUD file
def get_mud_file(url, device_id):
    url=url + "/mud"
    try:
        response = requests.get(url)
        if response.status_code == 200:
            inventory.store_mud(device_id, response.content)
            if not verify_mud_file(response.content):
                print(f"MUD file retrieved for device ID {device_id} is invalid.")
                return
            inventory.store_mud_policies(device_id, parse_mud(inventory.get_mud(device_id)))
            print(f"MUD file retrieved successfully for device ID {device_id}")
        else:
            print(f"Failed to retrieve MUD file for device ID {device_id}. HTTP status code: {response.status_code}")
    except requests.RequestException as e:
        print(f"An error occurred while fetching the MUD file for device ID {device_id}: {e}")

def verify_mud_file(mud_file):
    #todo implement MUD file verification
    return True

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)