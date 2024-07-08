from flask import jsonify
import subprocess


def enforce_ip_table(ip_table):
    for command in ip_table:
        result = subprocess.run(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print(result.stdout.decode('utf-8'), result.stderr.decode('utf-8'))
    return jsonify({'message': 'IP tables rules enforced'}), 200

#Function to convert policies to iptables rules
def translate_to_iptables(policies):
    iptables_rules = []
    for policy in policies:
        for port in policy['ports']:
            rule = f"sudo iptables -A {policy['direction'].upper()} -p {policy['protocol']} --dport {port} -j " + "ACCEPT" if {policy['action'].upper()=="allow"} else "DROP"
            iptables_rules.append(rule)
    return iptables_rules