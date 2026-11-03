#!/usr/bin/env python3
"""Kartin durumunu tek komutla okur: Slot A, Slot B ve metadata'nin son kaydi.

Karti RESETLEMEDEN okur (mode=HOTPLUG), yani deneme/onay mantigini bozmaz.

kullanim:
    python state.py                       # karttan okur (STM32_Programmer_CLI gerekir)
    python state.py --cli "C:\\yol\\STM32_Programmer_CLI.exe"
    python state.py --files meta.bin a_basligi.bin b_basligi.bin   # kartsiz, dosyadan
"""
import argparse
import os
import shutil
import struct
import subprocess
import sys
import tempfile
import zlib

SLOT_A   = 0x08020000
SLOT_B   = 0x08080000
META     = 0x08010000
META_LEN = 0x10000
HDR_LEN  = 0x60            # baslik (32) + imza (64)

IMAGE_MAGIC = 0x544F4F42
META_MAGIC  = 0x4154454D
REC         = 32
STATES = {0: "NONE", 1: "CONFIRMED", 2: "PENDING", 3: "TESTING",
          4: "SWAPPING", 5: "REVERTING"}

DEFAULT_CLI_PATHS = [
    r"C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe",
    r"C:\Program Files (x86)\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe",
]


def ver_text(v):
    return "%d.%d.%d" % (v >> 16, (v >> 8) & 0xFF, v & 0xFF)


def find_cli(user_path):
    if user_path:
        return user_path
    found = shutil.which("STM32_Programmer_CLI") or shutil.which("STM32_Programmer_CLI.exe")
    if found:
        return found
    for p in DEFAULT_CLI_PATHS:
        if os.path.isfile(p):
            return p
    raise SystemExit("hata: STM32_Programmer_CLI bulunamadi. --cli ile yolunu ver.")


def read_mem(cli, addr, size, path):
    cmd = [cli, "-c", "port=SWD", "mode=HOTPLUG", "-r", "0x%08X" % addr, "0x%X" % size, path]
    res = subprocess.run(cmd, capture_output=True, text=True)
    if res.returncode != 0 or not os.path.isfile(path):
        raise SystemExit("hata: kart okunamadi (%s):\n%s" % (hex(addr), (res.stdout + res.stderr)[-800:]))
    return open(path, "rb").read()


def describe_slot(name, data):
    if len(data) < HDR_LEN:
        return "%s: veri eksik" % name
    magic, hver, size, crc, fw = struct.unpack_from("<5I", data, 0)
    if magic != IMAGE_MAGIC:
        return "%s: imaj yok (magic 0x%08X)" % (name, magic)
    sig = data[0x20:0x60]
    sig_txt = "BOS (imzasiz)" if sig == b"\xff" * 64 else "var (ilk bayt %s)" % sig[:4].hex()
    return "%s: surum %s (0x%08X)  govde %d bayt  crc 0x%08X  imza %s" % (
        name, ver_text(fw), fw, size, crc, sig_txt)


def describe_meta(data):
    rows, bad, used = [], 0, 0
    for i in range(len(data) // REC):
        raw = data[i * REC:(i + 1) * REC]
        if raw == b"\xff" * REC:
            break
        used += 1
        magic, seq, state, _b, step, trial, minv, crc = struct.unpack("<8I", raw)
        ok = (magic == META_MAGIC) and ((zlib.crc32(raw[:28]) & 0xFFFFFFFF) == crc)
        if ok:
            rows.append((i, seq, state, step, trial, minv))
        else:
            bad += 1
    if used == len(data) // REC:
        print("UYARI: bos slot bulunamadi, okuma penceresi yetmemis olabilir (son kayit yanlis olabilir)")
    if not rows:
        return ["metadata: gecerli kayit yok"]
    i, seq, state, step, trial, minv = rows[-1]
    minv = 0 if minv == 0xFFFFFFFF else minv
    return [
        "metadata: %d slot dolu (%d gecerli, %d bozuk/yarim)" % (used, len(rows), bad),
        "  son kayit : slot %d  seq %d  durum %s  trial %d  swap_step %d" % (
            i, seq, STATES.get(state, "?%d" % state), trial, step),
        "  min_version: %s (0x%08X)  -> bundan eski imaj kabul edilmez" % (ver_text(minv), minv),
    ]

def get_state(cli):
    """Kartin durumunu sozluk olarak dondurur (karti resetlemeden okur)."""
    with tempfile.TemporaryDirectory() as tmp:
        meta = read_mem(cli, META, META_LEN, os.path.join(tmp, "meta.bin"))
        sa = read_mem(cli, SLOT_A, HDR_LEN, os.path.join(tmp, "a.bin"))
        sb = read_mem(cli, SLOT_B, HDR_LEN, os.path.join(tmp, "b.bin"))
    last = None
    for i in range(len(meta) // REC):
        raw = meta[i * REC:(i + 1) * REC]
        if raw == b"\xff" * REC:
            break
        magic, seq, state, _b, step, trial, minv, crc = struct.unpack("<8I", raw)
        if magic == META_MAGIC and (zlib.crc32(raw[:28]) & 0xFFFFFFFF) == crc:
            last = (seq, state, step, trial, 0 if minv == 0xFFFFFFFF else minv)
    if last is None:
        raise SystemExit("hata: metadata'da gecerli kayit yok")

    def slot_ver(d):
        magic, _h, _s, _c, fw = struct.unpack_from("<5I", d, 0)
        return fw if magic == IMAGE_MAGIC else None

    return {"a": slot_ver(sa), "b": slot_ver(sb), "seq": last[0],
            "state": STATES.get(last[1], "?%d" % last[1]),
            "trial": last[3], "min_version": last[4]}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--cli")
    ap.add_argument("--files", nargs=3, metavar=("META", "A", "B"))
    a = ap.parse_args()

    if a.files:
        meta, sa, sb = (open(p, "rb").read() for p in a.files)
    else:
        cli = find_cli(a.cli)
        with tempfile.TemporaryDirectory() as tmp:
            meta = read_mem(cli, META, META_LEN, os.path.join(tmp, "meta.bin"))
            sa = read_mem(cli, SLOT_A, HDR_LEN, os.path.join(tmp, "a.bin"))
            sb = read_mem(cli, SLOT_B, HDR_LEN, os.path.join(tmp, "b.bin"))

    print(describe_slot("Slot A (calisan)", sa))
    print(describe_slot("Slot B (yedek)  ", sb))
    for line in describe_meta(meta):
        print(line)
    return 0


if __name__ == "__main__":
    sys.exit(main())