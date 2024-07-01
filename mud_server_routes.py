from flask import Blueprint
from flask import jsonify
from mud_server_service import get_mud_file

bp = Blueprint('routes', __name__)

#Endpoint to retrieve MUD file for a specific device
#If the MUD file is not found in the inventory, the server will attempt to fetch it from the MUD File server
@bp.route('/mud/<device_id>/<mud_file_url>', methods=['GET'])
def get_mud(mud_file_url, device_id):
    mud = inventory.get_mud(device_id)
    if mud:
        return jsonify({'mud': mud}), 200
    else:
        get_mud_file(mud_file_url, device_id)
        if mud:
            return jsonify({'mud': mud}), 200
        else:
            return jsonify({'error': 'MUD not found'}), 404