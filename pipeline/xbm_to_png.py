#!/usr/bin/env python3
"""
xbm_to_png.py — Convert Griljor XBM bitmap files to PNG.

Reads all files in bitmaps/, bit/, and facebits/ and writes PNGs to
pipeline/out/sprites/{dir}/{name}.png

XBM format: text-based C header with hex byte array, bits LSB-first.
  1 bit = black (foreground), 0 bit = white (background).
Output PNG: 8-bit grayscale (0=black, 255=white).
"""

import os, re, struct, zlib, sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_BASE = os.path.join(REPO, 'pipeline', 'out', 'sprites')
SRC_DIRS = ['bitmaps', 'bit', 'facebits']


def write_png_gray(path, width, height, pixels):
    """Write 8-bit grayscale PNG. pixels: flat list of ints 0-255, row-major."""
    def chunk(tag, data):
        crc = zlib.crc32(tag + data) & 0xFFFFFFFF
        return struct.pack('>I', len(data)) + tag + data + struct.pack('>I', crc)

    raw = b''.join(b'\x00' + bytes(pixels[y * width:(y + 1) * width])
                   for y in range(height))

    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n')
        f.write(chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 0, 0, 0, 0)))
        f.write(chunk(b'IDAT', zlib.compress(raw, 6)))
        f.write(chunk(b'IEND', b''))


def parse_xbm(path):
    """Parse XBM file. Returns (width, height, pixels) where pixels is list of 0/255."""
    with open(path, 'r', errors='replace') as f:
        text = f.read()

    m_w = re.search(r'#define\s+\w+_width\s+(\d+)', text)
    m_h = re.search(r'#define\s+\w+_height\s+(\d+)', text)
    if not m_w or not m_h:
        raise ValueError('missing width/height defines')

    width  = int(m_w.group(1))
    height = int(m_h.group(1))

    hex_vals = re.findall(r'0x([0-9a-fA-F]{1,2})', text)
    if not hex_vals:
        raise ValueError('no hex data found')
    data = [int(v, 16) for v in hex_vals]

    row_bytes = (width + 7) // 8
    pixels = []
    for y in range(height):
        for x in range(width):
            byte_idx = y * row_bytes + (x // 8)
            bit = (data[byte_idx] >> (x % 8)) & 1
            pixels.append(0 if bit else 255)   # 1=black, 0=white

    return width, height, pixels


def convert_dir(src_dir, out_dir):
    converted, skipped = 0, 0
    for name in sorted(os.listdir(src_dir)):
        src = os.path.join(src_dir, name)
        if not os.path.isfile(src):
            continue
        out = os.path.join(out_dir, name + '.png')
        try:
            w, h, pixels = parse_xbm(src)
            write_png_gray(out, w, h, pixels)
            print(f'  {name:<30} {w}x{h}')
            converted += 1
        except Exception as e:
            print(f'  SKIP {name}: {e}', file=sys.stderr)
            skipped += 1
    return converted, skipped


def main():
    total_ok, total_skip = 0, 0
    for dirname in SRC_DIRS:
        src = os.path.join(REPO, dirname)
        if not os.path.isdir(src):
            print(f'  (skipping {dirname}/ — not found)')
            continue
        out = os.path.join(OUT_BASE, dirname)
        print(f'\n{dirname}/')
        ok, skip = convert_dir(src, out)
        total_ok += ok
        total_skip += skip

    print(f'\n{total_ok} PNGs written to {OUT_BASE}')
    if total_skip:
        print(f'{total_skip} files skipped (see stderr)')


if __name__ == '__main__':
    main()
