from Crypto.PublicKey import RSA

def generate_keys():
    # Generate RSA key pair
    key = RSA.generate(2048)
    
    # Extract the components
    private_key = key.export_key()
    public_key = key.publickey().export_key()
    
    # Return as tuple
    return public_key, private_key