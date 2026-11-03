#!/usr/bin/env python3
"""Ham uygulama .bin dosyasindan tek komutla imzali, dogrulanmis imaj uretir.

kullanim:
    python release.py <ham.bin> <major.minor.patch> [--key signing_key.pem]
                      [--pubkey boot_public_key.h] [--out cikti.bin]

Ornek:
    python release.py app_test.bin 7.0.0
    -> signed_7.0.0.bin   (fw_version = 0x00070000)
"""
import argparse
import hashlib
import os
import struct
import subprocess
import sys

IMAGE_MAGIC = 0x544F4F42
HERE = os.path.dirname(os.path.abspath(__file__))


def parse_version(text):
    parts = text.split(".")
    if len(parts) != 3 or not all(p.isdigit() for p in parts):
        raise SystemExit("hata: surum major.minor.patch bicimde olmali (ornek 7.0.0)")
    major, minor, patch = (int(p) for p in parts)
    if major > 0xFFFF or minor > 0xFF or patch > 0xFF:
        raise SystemExit("hata: major <= 65535, minor ve patch <= 255 olmali")
    return (major << 16) | (minor << 8) | patch


def run(script, *args):
    cmd = [sys.executable, os.path.join(HERE, script), *args]
    res = subprocess.run(cmd, capture_output=True, text=True)
    out = (res.stdout + res.stderr).strip()
    if res.returncode != 0:
        raise SystemExit(f"hata: {script} basarisiz ({res.returncode}):\n{out}")
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("raw")
    ap.add_argument("version")
    ap.add_argument("--key", default="signing_key.pem")
    ap.add_argument("--pubkey", default="boot_public_key.h")
    ap.add_argument("--out")
    a = ap.parse_args()

    fw = parse_version(a.version)
    out = a.out or f"signed_{a.version}.bin"

    for path in (a.raw, a.key, a.pubkey):
        if not os.path.isfile(path):
            raise SystemExit(f"hata: dosya yok: {path}")

    raw = open(a.raw, "rb").read()
    if len(raw) < 8:
        raise SystemExit("hata: girdi cok kucuk, ham uygulama .bin dosyasi mi?")
    if struct.unpack_from("<I", raw, 0)[0] == IMAGE_MAGIC:
        raise SystemExit("hata: girdide 'BOOT' basligi var, bu zaten bir imaj.\n"
                         "      Basliksiz HAM .bin dosyasini ver (CubeIDE Debug klasorundeki).")

    info = run("make_image.py", a.raw, out, hex(fw), a.key)
    check = run("verify_image.py", out, a.pubkey)
    if "GECERLI" not in check or "GECERSIZ" in check:
        raise SystemExit(f"hata: uretilen imaj dogrulanamadi: {check}")

    data = open(out, "rb").read()
    print(f"cikti      : {out}")
    print(f"surum      : {a.version}  (fw_version = 0x{fw:08X})")
    print(f"dosya      : {len(data)} bayt  (govde {len(data) - 512} + baslik 512)")
    print(info.splitlines()[0])
    print(f"dosya sha256: {hashlib.sha256(data).hexdigest()}")
    print(f"dogrulama  : {check}")
    return 0


if __name__ == "__main__":
    sys.exit(main())