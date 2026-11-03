# STM32F407 Secure Bootloader (bare-metal, USB DFU)

A bare-metal bootloader for the **STM32F407VGT Discovery** board, built step by step as a learning project.
No HAL: only CMSIS register headers. The firmware is updated over the board's own **USB OTG (CN5)** using
standard `dfu-util`, with power-fail-safe installation, automatic rollback, a watchdog, and signed images.

> **Status:** educational project, tested on the STM32F407 Discovery only. Not security-audited.

## Features

| Area | What it does |
|---|---|
| Update channel | USB DFU device (TinyUSB 0.21.0), no extra hardware |
| Staging | New image is written to Slot B first; the running image (Slot A) is never touched until the new one is verified |
| Power-fail-safe install | Sector-by-sector swap through a scratch sector; progress is logged in flash and resumes after a power cut |
| Trial boot + rollback | New image runs once in "testing" state; it must confirm itself, otherwise the bootloader swaps the old image back |
| Watchdog | IWDG is started right before the application runs; a hung application triggers a reset and therefore a rollback |
| Authenticity | ECDSA P-256 signature over SHA-256 (micro-ecc), public key embedded in the bootloader |
| Anti-rollback | Images older than the last confirmed version are rejected |
| Integrity | CRC32 over the image body |

## Memory map (1 MB flash)

| Region | Address | Size | Content |
|---|---|---|---|
| Bootloader | `0x08000000` | 64 KB | this project (sectors 0-3) |
| Metadata | `0x08010000` | 64 KB | append-only state log (sector 4) |
| Slot A | `0x08020000` | 384 KB | running application (sectors 5-7) |
| Slot B | `0x08080000` | 384 KB | staging / previous image (sectors 8-10) |
| Scratch | `0x080E0000` | 128 KB | temporary space for the swap (sector 11) |

Each slot starts with a 512-byte header, the application is linked at `slot start + 0x200`.

## Image format

```
0x00  magic 'BOOT' | header_version | image_size | image_crc32 | fw_version | reserved x3   (32 bytes, signed)
0x20  ECDSA P-256 signature, r || s (64 bytes)
0x60  0xFF padding up to 0x200
0x200 application body
```
The signature covers `SHA-256(header[0..31] || body)`, so the version number is protected too.
`fw_version = major << 16 | minor << 8 | patch`.

## Repository layout

```
STM-BOOTLOADER-UPDATED/
  BOOTLOADER-CODE/     bootloader (STM32CubeIDE project)
    config/            memory map, TinyUSB config, shared app<->bootloader contract, public key
    drivers/           clock, GPIO, SysTick, flash, USB, IWDG, RTC backup registers
    bsp/               board (LEDs, button)
    services/          image check, CRC32, SHA-256, signature, metadata log, swap, policy, DFU
    middleware/        TinyUSB (MIT), micro-ecc (BSD-2) - see the LICENSE files inside
  App_Main/            test application (blink + confirm + watchdog feed)
tools/                 Python host tools
```
Dependencies only point downwards: `main.c` -> services -> drivers/bsp -> CMSIS.

## Build and flash the bootloader

1. Open `BOOTLOADER-CODE` in **STM32CubeIDE** (developed with 1.11.0, arm-none-eabi-gcc 14.3.1).
2. Make sure `middleware` is a source location and `middleware/micro-ecc` is on the include path.
3. Build and flash through the ST-LINK (CN1). **Never use mass erase** when flashing, it would wipe the slots.

Never write an application image to `0x08000000`, that is where the bootloader lives.

## Your own signing key (required)

The public key in `config/boot_public_key.h` belongs to the author. You cannot create images for it.
Create your own pair:

```
cd tools
python gen_keys.py          # creates signing_key.pem (KEEP SECRET) and boot_public_key.h
```
Copy `boot_public_key.h` into the bootloader's `config/` folder and rebuild. **Never commit `signing_key.pem`.**

## Building and installing an application

Applications are linked for Slot A (`0x08020200`) and must set `SCB->VTOR` to that address.

```
pip install cryptography
python release.py app.bin 1.0.0          # raw .bin -> signed_1.0.0.bin, verified
```
Enter update mode (hold the **blue user button**, press **RESET**, release the button, blue LED on), connect USB (CN5):

```
dfu-util -d cafe:4000 -a 0 -D signed_1.0.0.bin
```
The board reboots, installs the image (orange LED blinks during the swap), and starts it. The application must
confirm itself (see `App_Main`) and feed the watchdog, otherwise it is rolled back.

## Host tools (`tools/`)

| Tool | Purpose |
|---|---|
| `gen_keys.py` | Generate the signing key pair and the C header with the public key |
| `make_image.py` | Add the header (and signature) to a raw `.bin` |
| `verify_image.py` | Verify an image against the public key header |
| `release.py` | One-command build of a signed, verified image (refuses inputs that already have a header) |
| `state.py` | Read slots and the metadata log from a running board without resetting it (needs STM32CubeProgrammer CLI) |
| `regress.py` | Semi-automatic test of the security gates: unsigned, forged version, corrupt body, old version, valid update |

## Known limitations

- USB VID/PID `0xCAFE:0x4000` are for testing only, use your own for a product.
- Flash write protection (WRP) and readout protection (RDP) are documented but **not enabled** by default,
  they make development harder. Enable them on the final unit. Never use RDP level 2 (irreversible).
- A full swap takes about 12 seconds because all three sectors are swapped every time.

## License

The project's own code is released under the [MIT License](LICENSE).
Third-party code in `middleware/` keeps its own license: TinyUSB (MIT) and micro-ecc (BSD 2-Clause),
see the `LICENSE` files in their folders.