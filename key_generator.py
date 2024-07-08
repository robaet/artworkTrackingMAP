from Crypto.PublicKey import RSA
from hashlib import sha512

def generate_keys():
    keyPair = RSA.generate(bits=1024)
    print(f"Public key:  (n={hex(keyPair.n)}, e={hex(keyPair.e)})")
    print(f"Private key: (n={hex(keyPair.n)}, d={hex(keyPair.d)})")
    return keyPair

def sign_mudfile(msg, keyPair):
    # RSA sign the message
    hash = int.from_bytes(sha512(msg).digest(), byteorder='big')
    signature = pow(hash, keyPair.d, keyPair.n)
    print("Signature:", hex(signature))

    return signature
def verify_mud_file(mud, signature, keyPair):
    # RSA verify signature
    mud = b'A messagef for signing'
    hash = int.from_bytes(sha512(mud).digest(), byteorder='big')
    hashFromSignature = pow(signature, keyPair.e, keyPair.n)
    print("Signature valid:", hash == hashFromSignature)

def main():
    keyPair = generate_keys()
    msg = b'A message for signing'
    sign_mudfile(msg, keyPair)
    verify_mud_file(msg, sign_mudfile(msg, keyPair), keyPair)


main()