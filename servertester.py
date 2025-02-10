import requests

url = "http://192.168.1.145:6000"
# GET request
response = requests.get(url)
print("GET Response:", response.text, response.status_code)

# POST request
data = '{"time": "1000001","temperature": "20C","humidity": "17%","acceleration": "[2, 3, 4]"}'
response = requests.post(url, data)
print("POST Response:", response.text)