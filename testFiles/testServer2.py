from flask import Flask, jsonify

app = Flask(__name__)

@app.route('/test/<device_id>', methods=['GET'])
def testconn(device_id):
    print("reached 5000 GET!!")
    return jsonify(message='Hello from server on port 5000!'), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)