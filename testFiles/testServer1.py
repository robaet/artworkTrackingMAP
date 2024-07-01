from flask import Flask, jsonify
import requests

app = Flask(__name__)

@app.route('/test/<device_id>', methods=['GET'])
def testconn(device_id):
    print("reached 6000 GET!!")
    url = f"http://127.0.0.1:5000/test/{device_id}"
    print(url)

    response = requests.get(url)
    print(response)

    return jsonify(message='Hello from server on port 6000!'), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=6000)