from flask import jsonify
import subprocess


def enforce_ip_table(ip_table):
    for command in ip_table:
        result = subprocess.run(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print(result.stdout.decode('utf-8'), result.stderr.decode('utf-8'))
    return jsonify({'message': 'IP tables rules enforced'}), 200