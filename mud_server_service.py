import logging
import requests

class Mud_Server_Service:

    def __init__(self, inventory):
        self.inventory = inventory

    #Function to parse MUD file and extract policies
    def parse_mud(self, mud):
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
    def translate_to_iptables(self, policies):
        iptables_rules = []
        for policy in policies:
            for port in policy['ports']:
                rule = f"iptables -A {policy['direction'].upper()} -p {policy['protocol']} --dport {port} -j {policy['action'].upper()}"
                iptables_rules.append(rule)
        return iptables_rules

    def verify_mud_file(self, mud_file):
        #todo implement MUD file verification
        return True

    #Function to fetch MUD file from the MUD File server
    #todo periodically check for updates to the MUD file
    def get_mud_file(self, url, device_id):
        logger = logging.getLogger(__name__)
        logging.basicConfig(filename='mudfile_retrieval_log.log', encoding='utf-8', level=logging.DEBUG)
        url =url + "/mud"

        headers = {
            'Accept': 'application/mud+json',
            'Accept-Language': 'en-US',
            'User-Agent': 'MUDManager/1.0'
        }
        
        try:
            response = requests.get(url, headers=headers, allow_redirects=True)
            if response.status_code == 200:
                self.inventory.store_mud(device_id, response.content)
                if not self.verify_mud_file(response.content):
                    logging.error(f"MUD file retrieved for device ID {device_id} is invalid.")
                    return
                self.inventory.store_mud_policies(device_id, parse_mud(self.inventory.get_mud(device_id)))
                logging.info(f"MUD file retrieved successfully for device ID {device_id}")

            elif response.status_code >= 300 and response.status_code < 400:
                # Automatically follow redirects
                redirect_url = response.headers.get('Location')
                if redirect_url:
                    self.get_mud_file(redirect_url, device_id)
                else:
                    logging.error(f"Failed to retrieve MUD file for device ID {device_id}. Redirect URL not found.")
            else:
                logging.error(f"Failed to retrieve MUD file for device ID {device_id}. HTTP status code: {response.status_code}")
        except requests.RequestException as e:
            logging.error(f"An error occurred while fetching the MUD file for device ID {device_id}: {e}")