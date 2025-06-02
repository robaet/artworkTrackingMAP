from flask import Flask, request, jsonify
import requests
import json
from apscheduler.schedulers.background import BackgroundScheduler
import atexit
import socket

ALLOWED_IPS = {'192.168.1.100', '10.0.0.1', '13.38.251.115', '127.0.0.1', '192.168.1.145', '192.168.1.120', '188.61.197.109', '213.55.241.102'}
import signal
import sys

def signal_handler(sig, frame):
    print('Exiting gracefully...')
    sys.exit(0)

def dataHandlerSocket():
    print("Starting data handler server...")
    HOST = '0.0.0.0'
    PORT = 5000
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        server_socket.settimeout(1.0)  # Set timeout to 1 second
        server_socket.bind((HOST, PORT))
        server_socket.listen(5)

        print(f"Server listening on {HOST}:{PORT}")

        signal.signal(signal.SIGINT, signal_handler)  # Register signal handler
        while True:
            try:
                conn, addr = server_socket.accept()
            except socket.timeout:
                continue  # Timeout reached, continue to allow signal handling

            with conn:
                print('Connected by', addr)
                data = conn.recv(1024)
                if not data:
                    break
                print('Received', repr(data))
                conn.sendall(data)



if __name__ == '__main__':
    print("Starting bootstrapping server...")
    dataHandlerSocket()
