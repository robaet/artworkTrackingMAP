import subprocess

def generate_keys():
    private_key = subprocess.run(
        ["openssl", "genpkey", "-algorithm", "RSA"],
        capture_output=True,
        check=True,
        text=True
    ).stdout

    # Generate public key from the private key
    public_key = subprocess.run(
        ["openssl", "rsa", "-pubout"],
        input=private_key,
        capture_output=True,
        check=True,
        text=True
    ).stdout

    return private_key, public_key