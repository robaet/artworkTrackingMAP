from flask import Flask, request, jsonify
import subprocess


def enforce_ip_rule(command):
    result = subprocess.run(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return result.stdout.decode('utf-8'), result.stderr.decode('utf-8')

def add_rule(source_ip, port):
    command = f"sudo iptables -A INPUT -p tcp -s {source_ip} --dport {port} -j ACCEPT"
    stdout, stderr = enforce_ip_rule(command)

    if stderr:
        return jsonify({"error": stderr}), 400
    return jsonify({"message": f"Rule added: {stdout}"}), 200

def delete_rule(source_ip, port):
    command = f"sudo iptables -D INPUT -p tcp -s {source_ip} --dport {port} -j ACCEPT"
    stdout, stderr = enforce_ip_rule(command)

    if stderr:
        return jsonify({"error": stderr}), 400
    return jsonify({"message": f"Rule deleted: {stdout}"}), 200