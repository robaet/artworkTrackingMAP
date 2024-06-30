from flask import Flask, request, jsonify
import json

app = Flask(__name__)

ALLOWED_IPS = {'192.168.1.100', '10.0.0.1', '13.38.251.115', '127.0.0.1'}

def is_valid_ip(ip_address):
    # check if the IP address is in the allowed set
    return ip_address in ALLOWED_IPS

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)