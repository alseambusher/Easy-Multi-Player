import os
from hashlib import md5, sha1


# returns two keys private_key, public key
def api_key_gen():
    return sha1(os.urandom(1024)).hexdigest(), md5(os.urandom(1024)).hexdigest()


def session_key_gen():
    return md5(os.urandom(1024)).hexdigest()
