#!/usr/bin/env python3
"""Uygulama .bin dosyasinin basina 512 byte'lik imaj basligi ekler.
Anahtar dosyasi verilirse basliga ECDSA P-256 imzasi da yazar."""
import hashlib
import struct
import sys
import zlib

MAGIC          = 0x544F4F42
HEADER_VERSION = 1
HEADER_SIZE    = 0x200
SLOT_SIZE      = 0x60000
SIG_OFFSET     = 0x20          # imza, 32 baytlik alanlarin hemen arkasinda
SIG_SIZE       = 64


def sign(key_file, signed_data):
    from cryptography.hazmat.primitives import hashes, serialization
    from cryptography.hazmat.primitives.asymmetric import ec
    from cryptography.hazmat.primitives.asymmetric.utils import decode_dss_signature

    with open(key_file, "rb") as f:
        priv = serialization.load_pem_private_key(f.read(), password=None)

    der = priv.sign(signed_data, ec.ECDSA(hashes.SHA256()))
    r, s = decode_dss_signature(der)
    return r.to_bytes(32, "big") + s.to_bytes(32, "big")


def main():
    if len(sys.argv) not in (4, 5):
        print("kullanim: make_image.py <girdi.bin> <cikti.bin> <fw_versiyon> [signing_key.pem]")
        return 1

    src, dst, ver = sys.argv[1], sys.argv[2], int(sys.argv[3], 0)
    key_file = sys.argv[4] if len(sys.argv) == 5 else None

    with open(src, "rb") as f:
        body = f.read()

    body += b"\xff" * ((-len(body)) % 4)          # 4 byte'in katina tamamla

    if len(body) > SLOT_SIZE - HEADER_SIZE:
        print("hata: imaj slota sigmiyor")
        return 1

    crc = zlib.crc32(body) & 0xFFFFFFFF
    fields = struct.pack("<8I", MAGIC, HEADER_VERSION, len(body), crc, ver, 0, 0, 0)

    header = fields
    if key_file:
        sig = sign(key_file, fields + body)
        header = fields + sig
    header = header.ljust(HEADER_SIZE, b"\xff")   # 512 byte'a tamamla

    with open(dst, "wb") as f:
        f.write(header + body)

    digest = hashlib.sha256(fields + body).hexdigest()
    print(f"imaj: {len(body)} byte, crc32=0x{crc:08X}, fw=0x{ver:X}")
    if key_file:
        print(f"imzali: sha256={digest[:16]}...  imza={sig[:4].hex()}...")
    else:
        print("imzasiz")
    return 0


if __name__ == "__main__":
    sys.exit(main())