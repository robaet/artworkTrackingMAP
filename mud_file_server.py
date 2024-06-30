from flask import Flask, request, jsonify
import json
import requests

app = Flask(__name__)
ALLOWED_IPS = {'192.168.1.100', '10.0.0.1', '13.38.251.115', '127.0.0.1'}

def is_valid_ip(ip_address):
    # check if the IP address is in the allowed set
    return ip_address in ALLOWED_IPS

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

@app.route('/mud', methods=['GET'])
def return_mud_file(device_id, mud_file_url):
    if not is_valid_ip(request.remote_addr):
        return jsonify({'error': 'Unauthorized IP address'}), 403
    search_mud_file(mud_file_url, device_id, mud_server_IP)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=7000)