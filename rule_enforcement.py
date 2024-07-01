from flask import Flask, request, jsonify
import subprocess

app = Flask(__name__)

def run_iptables_command(command):
    result = subprocess.run(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return result.stdout.decode('utf-8'), result.stderr.decode('utf-8')

@app.route('/add_rule', methods=['POST'])
def add_rule():
    source_ip = request.json.get('source_ip')
    port = request.json.get('port')

    # Example command to allow traffic from a specific IP to a specific port
    command = f"sudo iptables -A INPUT -p tcp -s {source_ip} --dport {port} -j ACCEPT"
    stdout, stderr = run_iptables_command(command)

    if stderr:
        return jsonify({"error": stderr}), 400
    return jsonify({"message": f"Rule added: {stdout}"}), 200

@app.route('/delete_rule', methods=['POST'])
def delete_rule():
    source_ip = request.json.get('source_ip')
    port = request.json.get('port')

    # Example command to delete a specific rule
    command = f"sudo iptables -D INPUT -p tcp -s {source_ip} --dport {port} -j ACCEPT"
    stdout, stderr = run_iptables_command(command)

    if stderr:
        return jsonify({"error": stderr}), 400
    return jsonify({"message": f"Rule deleted: {stdout}"}), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
