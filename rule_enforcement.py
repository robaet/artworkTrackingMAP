from flask import jsonify
import subprocess


def enforce_ip_table(ip_table):
    #return "enforce_ip_table worked"
    for command in ip_table:
        try:
            result = subprocess.run(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            print(result.stdout.decode('utf-8'), result.stderr.decode('utf-8'))
        except subprocess.CalledProcessError as e:
            print(f"Error enforcing iptables rule: {e}")
    print("IP tables rules enforced.")
    return True

def translate_to_iptables(policies):
    #return "translate_to_iptables worked"
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

def delete_all_rules():
    #return "delete_all_rules worked"
    try:
        subprocess.run(['iptables', '-F'], check=True)
        print("All iptables rules deleted.")
    except subprocess.CalledProcessError as e:
        print(f"Error deleting iptables rules: {e}")

def get_current_iptables():
    #return "get_current_iptables worked"
    try:
        result = subprocess.run(['iptables', '-L', '-n', '-v'], capture_output=True, text=True, check=True)
        return result.stdout
    except subprocess.CalledProcessError as e:
        return f"Error running iptables: {e.stderr}"
    except FileNotFoundError:
        return "iptables command not found. Are you sure it's installed on your system?"
    
def remove_rules_with_certain_ip(ip_address):
    try:
        result = subprocess.run(
            ["iptables", "-S"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=True
        )
        
        rules = result.stdout.strip().split('\n')
        
        for rule in rules:
            if ip_address in rule:
                delete_rule = rule.replace("-A", "-D", 1)  # Change to delete
                print(f"Removing rule: {delete_rule}")
                subprocess.run(
                    ["iptables"] + delete_rule.split(),
                    check=True
                )
                print(f"Removed rule: {delete_rule}")

        get_current_iptables()
                
    except subprocess.CalledProcessError as e:
        print(f"Error occurred: {e.stderr}")
