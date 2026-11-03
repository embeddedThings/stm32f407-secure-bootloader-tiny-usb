#!/usr/bin/env python3
"""Yari otomatik regresyon testi: guvenlik kapilarini tek komutla dener.

Her senaryoda guncelleme moduna girmen istenir (mavi butonu basili tut, RESET'e bas,
mavi LED yanince birak, ENTER). Gerisini betik yapar: imaji uretir, dfu-util ile yollar,
karti RESETLEMEDEN okuyup sonucu kontrol eder.

kullanim:
    python regress.py <ham_saglikli_uygulama.bin> [--cli yol] [--dfu-util yol]

Not: ham uygulama onay vermeli (APP_CONFIRM_ENABLE 1) ve donmamali. Surum numaralari
kartin mevcut durumuna gore otomatik secilir, betik tekrar tekrar calistirilabilir.
"""
import argparse
import os
import subprocess
import sys
import time

import state as st

HERE = os.path.dirname(os.path.abspath(__file__))
KEY = "signing_key.pem"
MAJOR = 0x00010000


def build_image(raw, version, out, sign=True):
    cmd = [sys.executable, os.path.join(HERE, "make_image.py"), raw, out, hex(version)]
    if sign:
        cmd.append(KEY)
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        raise SystemExit("hata: imaj uretilemedi:\n" + r.stdout + r.stderr)


def flip(path, offset):
    d = bytearray(open(path, "rb").read())
    d[offset] ^= 0xFF
    open(path, "wb").write(d)


def dfu_send(dfu_util, path):
    r = subprocess.run([dfu_util, "-d", "cafe:4000", "-a", "0", "-D", path],
                       capture_output=True, text=True, timeout=180)
    return r.returncode == 0, (r.stdout + r.stderr)


def file_fw(path):
    d = open(path, "rb").read()
    return int.from_bytes(d[16:20], "little")


def run(raw, get_state, send, prompt, build=build_image, tamper=flip, wait=90, poll=2.0,
        sleep=time.sleep, workdir="."):
    s0 = get_state()
    if s0["state"] != "CONFIRMED" or s0["a"] is None:
        raise SystemExit("hata: kart temiz degil (durum %s, Slot A %s). Once bir RESET atip "
                         "onaylanmis bir uygulamaya don." % (s0["state"], s0["a"]))
    newer = max(s0["a"], s0["min_version"]) + MAJOR
    older = s0["min_version"] - 1 if s0["min_version"] > 0 else None
    results = []

    def p(name):
        return os.path.join(workdir, name)

    def expect_reject(title, path):
        # dfu-util cikis koduna GUVENMIYORUZ (basarida da hatada da yaniltici). Karara kartin
        # durumuna bakilir: dosya Slot B'ye ulasti mi, ama A / durum / min_version ayni mi?
        before = get_state()
        prompt(title)
        ok, _out = send(path)
        sleep(1.0)
        after = get_state()
        fw = file_fw(path)
        delivered = (after["b"] == fw)
        unchanged = (after["a"] == before["a"] and after["state"] == "CONFIRMED"
                     and after["min_version"] == before["min_version"])
        if not unchanged:
            results.append((title, False, "KABUL EDILDI veya durum degisti: oncesi %s -> sonrasi %s" % (before, after)))
        elif not delivered:
            results.append((title, False, "TEST GECERSIZ: dosya Slot B'ye ulasmadi (B=%s). "
                            "Guncelleme moduna girdin mi?" % (st.ver_text(after["b"]) if after["b"] is not None else "bos")))
        else:
            results.append((title, True, "reddedildi (dosya karta ulasti, kabul edilmedi) [dfu-util cikis kodu %s]"
                            % ("0" if ok else "sifirdan farkli")))

    build(raw, newer, p("t_unsigned.bin"), sign=False)
    expect_reject("1) imzasiz imaj reddedilmeli", p("t_unsigned.bin"))

    build(raw, newer, p("t_fakever.bin"))
    tamper(p("t_fakever.bin"), 0x10)
    expect_reject("2) sahte surum (imzali basliga dokunulmus) reddedilmeli", p("t_fakever.bin"))

    build(raw, newer, p("t_badbody.bin"))
    tamper(p("t_badbody.bin"), 0x300)
    expect_reject("3) bozuk govde reddedilmeli", p("t_badbody.bin"))

    if older is None:
        results.append(("4) eski surum (anti-rollback) reddedilmeli", None, "ATLANDI: min_version 0, daha eski surum uretilemez"))
    else:
        build(raw, older, p("t_older.bin"))
        expect_reject("4) imzali ama eski surum reddedilmeli", p("t_older.bin"))

    build(raw, newer, p("t_good.bin"))
    prompt("5) dogru imzali yeni surum (%s) kabul edilmeli" % st.ver_text(newer))
    ok, out = send(p("t_good.bin"))
    # Basarida da dfu-util hata kodu donebilir (kart kendini resetliyor); karari duruma gore ver.
    t0, s1 = time.time(), None
    while time.time() - t0 < wait:
        sleep(poll)
        try:
            s1 = get_state()
        except SystemExit:
            continue                       # kart takas sirasinda mesgul olabilir
        if s1["a"] == newer and s1["state"] == "CONFIRMED":
            break
    good = bool(s1) and s1["a"] == newer and s1["state"] == "CONFIRMED" and s1["min_version"] == newer
    results.append(("5) dogru imzali yeni surum", good,
                    "A=%s, durum CONFIRMED, min_version=%s [dfu-util cikis kodu %s]"
                    % (st.ver_text(newer), st.ver_text(newer), "0" if ok else "sifirdan farkli")
                    if good else "beklenen olmadi: %s" % s1))

    print("\n==== SONUC ====")
    failed = 0
    for title, ok, note in results:
        mark = "ATLANDI" if ok is None else ("GECTI  " if ok else "KALDI  ")
        failed += (ok is False)
        print("%s %s -> %s" % (mark, title, note))
    return failed

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("raw")
    ap.add_argument("--cli")
    ap.add_argument("--dfu-util", default="dfu-util")
    a = ap.parse_args()

    cli = st.find_cli(a.cli)

    def prompt(title):
        print("\n" + title)
        input("  Guncelleme moduna gir (mavi buton basili + RESET, mavi LED), USB takili, sonra ENTER: ")

    failed = run(a.raw, lambda: st.get_state(cli), lambda f: dfu_send(a.dfu_util, f), prompt)
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())