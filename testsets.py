from Device import Device

class Inventory:
    def __init__(self):
        self.devices = set()

    def set_devices(self, device: Device):
        self.devices.add(device)

    def get_devices(self):
        return self.devices
    def remove_device(self, device: Device):
        print(f"Device with ID {device.getId()} removed from inventory.")
        self.devices.remove(device)

inventory = Inventory()

inventory.set_devices(Device("1", "cert1", "mud_url1", ""))
inventory.set_devices(Device("2", "cert2", "mud_url2", ""))
inventory.set_devices(Device("3", "cert3", "mud_url3", ""))
inventory.set_devices(Device("4", "cert4", "mud_url4", ""))
inventory.set_devices(Device("5", "cert5", "mud_url5", ""))

def duo():
    a = []

    for device in inventory.get_devices():
        if device.getId() == "1":
            a.append(device)
    for x in a:
        inventory.remove_device(x)
    
    for x in inventory.get_devices():
        print(x.getId())

duo()