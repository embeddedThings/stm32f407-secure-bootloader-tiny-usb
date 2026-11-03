#!/usr/bin/env python3
"""Imzali imaji, bootloader'in kullanacagi acik anahtarla dogrular."""
import re
import struct
import sys

from cryptography.exceptions import InvalidSignature
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.asymmetric.utils import encode_dss_signature

HEADER_SIZE = 0x200
SIG_OFFSET  = 0x20
SIG_SIZE    = 64


def load_public_key(h_file):
    text = open(h_file, encoding="utf-8").read()
    raw = bytes(int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{2})", text))
    if len(raw) != 64:
        raise SystemExit(f"hata: acik anahtar {len(raw)} bayt, 64 olmali")
    x = int.from_bytes(raw[:32], "big")
    y = int.from_bytes(raw[32:], "big")
    return ec.EllipticCurvePublicNumbers(x, y, ec.SECP256R1()).public_key()


def main():
    if len(sys.argv) != 3:
        print("kullanim: verify_image.py <imaj.bin> <boot_public_key.h>")
        return 1

    data = open(sys.argv[1], "rb").read()
    pub = load_public_key(sys.argv[2])

    fields = data[:0x20]
    sig    = data[SIG_OFFSET:SIG_OFFSET + SIG_SIZE]
    size   = struct.unpack_from("<I", data, 8)[0]
    body   = data[HEADER_SIZE:HEADER_SIZE + size]

    if sig == b"\xff" * SIG_SIZE:
        print("IMZA YOK")
        return 2

    der = encode_dss_signature(int.from_bytes(sig[:32], "big"),
                               int.from_bytes(sig[32:], "big"))
    try:
        pub.verify(der, fields + body, ec.ECDSA(hashes.SHA256()))
    except InvalidSignature:
        print("IMZA GECERSIZ")
        return 3

    print("IMZA GECERLI")
    return 0


if __name__ == "__main__":
    sys.exit(main())