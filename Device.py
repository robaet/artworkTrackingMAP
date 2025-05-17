class Device:
    def __init__(self, id, certificate, mud_url, ip_address):
        self.id = id
        self.certificate = certificate
        self.mud_url = mud_url
        self.ip_address = ip_address

    def getId(self):
        return self.id
    
    def getCertificate(self):
        return self.certificate
    
    def getMudUrl(self):
        return self.mud_url
    
    def getIpAddress(self):
        return self.ip_address

    def __str__(self):
        return f"Device(name={self.id}, type={self.certificate}, status={self.mud_url})"