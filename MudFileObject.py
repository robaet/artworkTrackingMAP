import json
class MudFileObject:
    def __init__(self, name, manufacturer, model, policy, cache_validity, mud_url, last_update):
        self.name = name
        self.manufacturer = manufacturer
        self.model = model
        self.policy = policy
        self.cache_validity = cache_validity
        self.mud_url = mud_url
        self.last_update = last_update