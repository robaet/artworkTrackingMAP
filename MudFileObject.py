import json
class MudFileObject:
    def __init__(self, name, manufacturer, model, cache_validity, mud_url, last_update, inbound, outbound):
        self.name = name
        self.manufacturer = manufacturer
        self.model = model
        self.cache_validity = cache_validity
        self.mud_url = mud_url
        self.last_update = last_update
        self.inbound = inbound
        self.outbound = outbound