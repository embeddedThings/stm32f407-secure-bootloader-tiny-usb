import os
import sys
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import serialization

KEY_FILE = "signing_key.pem"
HDR_FILE = "boot_public_key.h"

if os.path.exists(KEY_FILE):
    sys.exit(KEY_FILE + " zaten var, uzerine yazmiyorum (eski imzalar gecersiz olur).")

priv = ec.generate_private_key(ec.SECP256R1())

with open(KEY_FILE, "wb") as f:
    f.write(priv.private_bytes(serialization.Encoding.PEM,
                               serialization.PrivateFormat.PKCS8,
                               serialization.NoEncryption()))

n = priv.public_key().public_numbers()
raw = n.x.to_bytes(32, "big") + n.y.to_bytes(32, "big")   # 64 bayt: X || Y

lines = []
for i in range(0, 64, 8):
    lines.append("    " + ", ".join("0x%02X" % b for b in raw[i:i + 8]) + ",")

with open(HDR_FILE, "w") as f:
    f.write("#ifndef BOOT_PUBLIC_KEY_H\n#define BOOT_PUBLIC_KEY_H\n\n")
    f.write("/* ECDSA P-256 acik anahtar (X || Y, 64 bayt). Gizli degildir. */\n")
    f.write("static const unsigned char BOOT_PUBLIC_KEY[64] = {\n")
    f.write("\n".join(lines) + "\n};\n\n#endif\n")

print("Olusturuldu:", KEY_FILE, "ve", HDR_FILE)