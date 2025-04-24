class Device:
    def __init__(self, id, certificate, mud_url):
        self.id = id
        self.certificate = certificate
        self.mud_url = mud_url

    def __str__(self):
        return f"Device(name={self.id}, type={self.certificate}, status={self.mud_url})"