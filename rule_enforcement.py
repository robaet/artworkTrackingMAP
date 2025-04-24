from flask import jsonify
import subprocess


def enforce_ip_table(ip_table):
    for command in ip_table:
        result = subprocess.run(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print(result.stdout.decode('utf-8'), result.stderr.decode('utf-8'))
    return {'message': 'IP tables rules enforced'}

def translate_to_iptables(policies):
    iptables_rules = []
    for policy in policies:
        src_ports = policy.get('src-ports', [])
        dst_ports = policy.get('dst-ports', [])
        
        if not src_ports:
            src_ports = [None]
        if not dst_ports:
            dst_ports = [None]
        
        for src_port in src_ports:
            for dst_port in dst_ports:
                action = policy['action'].upper()
                if action == "ACCEPT":
                    action = "ACCEPT"
                elif action == "DROP":
                    action = "DROP"
                elif action == "REJECT":
                    action = "REJECT --reject-with tcp-reset"
                else:
                    raise ValueError(f"Unknown action: {policy['action']}")
                
                rule = f"sudo iptables -I {policy['direction'].upper()} -p {policy['protocol']} "
                
                if src_port is not None:
                    rule += f"--sport {src_port} "
                
                if dst_port is not None:
                    rule += f"--dport {dst_port} "
                
                rule += f"-j {action}"
                
                if 'src-ip' in policy:
                    rule += f" -s {policy['src-ip']}"
                
                if 'dst-ip' in policy:
                    rule += f" -d {policy['dst-ip']}"
                
                iptables_rules.append(rule)
    print("ip table rules: " + str(iptables_rules))
    return iptables_rules

