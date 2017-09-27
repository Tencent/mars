from binascii import hexlify, unhexlify

import pyelliptic

CURVE = 'secp256k1'

svr = pyelliptic.ECC(curve=CURVE)

svr_pubkey = svr.get_pubkey()
svr_privkey = svr.get_privkey()


print("save private key")

print(hexlify(svr_privkey))

print("\nappender_open's parameter:")
print("%s%s" %(hexlify(svr.pubkey_x),  hexlify(svr.pubkey_y)))

