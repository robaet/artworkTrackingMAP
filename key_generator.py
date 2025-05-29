from OpenSSL import crypto

def generate_key_pair():
    key = crypto.PKey()
    key.generate_key(crypto.TYPE_RSA, 2048)
    
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
    signature = crypto.sign(private_key, data, 'sha256')
    return signature

def verify_signature(data, signature, certificate_path):
    with open(certificate_path, 'rb') as f:
        pem_data = f.read()
    certificate = crypto.load_certificate(crypto.FILETYPE_PEM, pem_data)
    try:
        crypto.verify(certificate, signature, data, 'sha256')
        print("Signature verified")
        return True
    except crypto.Error:
        print("Signature verification failed")
        return False
