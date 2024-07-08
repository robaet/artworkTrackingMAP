from OpenSSL import crypto
import cryptography

def generate_key_pair():
    # Generate a key pair
    key = crypto.PKey()
    key.generate_key(crypto.TYPE_RSA, 2048)
    
    # Generate a self-signed certificate
    cert = crypto.X509()
    cert.get_subject().CN = "example.com"
    cert.set_serial_number(1000)
    cert.gmtime_adj_notBefore(0)
    cert.gmtime_adj_notAfter(10*365*24*60*60)
    cert.set_issuer(cert.get_subject())
    cert.set_pubkey(key)
    cert.sign(key, 'sha256')
    
    return key, cert

def sign_file(data, private_key):
    '''with open(file_path, 'rb') as f:
        data = f.read()
    '''
    # Create a signature using the private key
    #signature = cryptography.hazmat.primitives.asymmetric.rsa.generate_private_key(private_key, data, 'sha256')
    signature = crypto.sign(private_key, data, 'sha256')
    
    return signature

def verify_signature(data, signature, certificate):
    with open(file_path, 'rb') as f:
        data = f.read()
    
    try:
        # Verify the signature using the public key from the certificate
        crypto.verify(certificate, signature, data, 'sha256')
        print("Signature verified")
        return True
    except crypto.Error:
        print("Signature verification failed")
        return False

private_key, certificate = generate_key_pair()

file_path = 'signthis.txt'
signature = sign_file(file_path, private_key)

is_valid = verify_signature(file_path, signature, certificate)
print("Signature valid:", is_valid)
