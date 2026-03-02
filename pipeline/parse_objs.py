#!/usr/bin/env python3
"""
parse_objs.py — Parse Griljor .obj files into JSON.

.obj file format:
  1. Text header lines ending with the sentinel:
     "-end of object messages (do not remove this line)-"
     Header includes "okn_object_set_size:N" giving the object count.
  2. Binary section: N object records in "diag" format.

Diag record format (repeated fields until ENDLIST):
  2 bytes  field ID (big-endian: id[0]*256 + id[1])
  payload  depends on field type:
    BOLBOX   — 0 bytes (presence = TRUE)
    INTBOX   — 2 bytes sign-magnitude
    LINKPIC  — 2 bytes sign-magnitude (same encoding as INTBOX)
    LONGINT  — 4 bytes sign-magnitude
    STRBOX   — null-terminated string
    ICONPIC  — 128 bytes raw bitmap data
  ENDLIST: field ID = 0x0000 terminates the record.

Output: JSON per .obj file + PNG for each object's embedded bitmap/mask.
"""

import os, re, sys, json, struct, zlib

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_DIR = os.path.join(REPO, 'pipeline', 'out', 'data', 'objects')

END_OF_MESSAGES = '-end of object messages (do not remove this line)-'
BITMAP_ARRAY_SIZE = 128
ENDLIST_ID = 0

# Property table: {id: (name, type)}
# Derived from src/objprops.c AllocObjectProperties()
# Current properties
OBJ_PROPS = {
    100: ('bitmap', 'ICONPIC'),
    110: ('mask', 'ICONPIC'),
    120: ('type', 'INTBOX'),
    130: ('masked', 'BOLBOX'),
    140: ('recorded', 'BOLBOX'),
    151: ('lookmsg', 'STRBOX'),
    161: ('examinemsg', 'STRBOX'),
    181: ('usemsg', 'STRBOX'),
    191: ('roommsg', 'STRBOX'),
    201: ('allmsg', 'STRBOX'),
    221: ('usealtermsg', 'STRBOX'),
    231: ('roomaltermsg', 'STRBOX'),
    241: ('allaltermsg', 'STRBOX'),
    251: ('hitmsg', 'STRBOX'),
    261: ('victimmsg', 'STRBOX'),
    270: ('name', 'STRBOX'),
    280: ('color', 'STRBOX'),
    290: ('minplayers', 'INTBOX'),
    292: ('maxplayers', 'INTBOX'),
    318: ('scalebits', 'INTBOX'),
    319: ('represented', 'INTBOX'),
    330: ('class', 'INTBOX'),
    332: ('rooms', 'INTBOX'),
    334: ('teams', 'INTBOX'),
    340: ('minlevel', 'INTBOX'),
    350: ('maxlevel', 'INTBOX'),
    360: ('experience', 'INTBOX'),
    370: ('power', 'INTBOX'),
    380: ('mana', 'INTBOX'),
    385: ('strength', 'INTBOX'),
    390: ('health', 'INTBOX'),
    400: ('needanother', 'BOLBOX'),
    410: ('otherobj', 'INTBOX'),
    420: ('deity', 'BOLBOX'),
    450: ('takeable', 'BOLBOX'),
    460: ('weight', 'INTBOX'),
    490: ('lost', 'BOLBOX'),
    500: ('exit', 'BOLBOX'),
    510: ('movement', 'INTBOX'),
    520: ('permeable', 'BOLBOX'),
    530: ('transparent', 'BOLBOX'),
    540: ('concealing', 'BOLBOX'),
    550: ('glows', 'BOLBOX'),
    560: ('flashlight', 'INTBOX'),
    570: ('invisible', 'BOLBOX'),
    580: ('connectgroup', 'INTBOX'),
    582: ('connectbits', 'INTBOX'),
    710: ('weapon', 'BOLBOX'),
    720: ('range', 'INTBOX'),
    730: ('arc', 'INTBOX'),
    740: ('spread', 'INTBOX'),
    750: ('fan', 'INTBOX'),
    760: ('refire', 'INTBOX'),
    770: ('stop', 'BOLBOX'),
    780: ('numbered', 'BOLBOX'),
    790: ('capacity', 'INTBOX'),
    800: ('movingobj', 'INTBOX'),
    810: ('charges', 'INTBOX'),
    820: ('directional', 'BOLBOX'),
    830: ('speed', 'INTBOX'),
    840: ('damage', 'INTBOX'),
    860: ('mowdown', 'BOLBOX'),
    870: ('ignoring', 'INTBOX'),
    880: ('piercing', 'INTBOX'),
    890: ('destroys', 'INTBOX'),
    910: ('explodes', 'INTBOX'),
    920: ('snuffs', 'BOLBOX'),
    930: ('boombit', 'INTBOX'),
    940: ('vulnerable', 'BOLBOX'),
    950: ('flammable', 'INTBOX'),
    960: ('destroyed', 'INTBOX'),
    970: ('opens', 'INTBOX'),
    975: ('hasid', 'BOLBOX'),
    980: ('swings', 'BOLBOX'),
    990: ('alternate', 'INTBOX'),
    1010: ('wearable', 'INTBOX'),
    1060: ('important', 'BOLBOX'),
    1070: ('flag', 'BOLBOX'),
    1080: ('flagteams', 'INTBOX'),
    1125: ('secret', 'BOLBOX'),
    1140: ('magic', 'INTBOX'),
    1152: ('ignoremove', 'BOLBOX'),
    1154: ('ignoretrans', 'BOLBOX'),
    1156: ('ignoreperm', 'BOLBOX'),
    1163: ('overridden', 'INTBOX'),
    1167: ('overrider', 'INTBOX'),
    1170: ('specialmsg', 'INTBOX'),
    1180: ('set0', 'BOLBOX'),
    1190: ('def0', 'INTBOX'),
    1200: ('set1', 'BOLBOX'),
    1210: ('def1', 'INTBOX'),
    1220: ('set2', 'BOLBOX'),
    1230: ('def2', 'INTBOX'),
    1240: ('set3', 'BOLBOX'),
    1250: ('def3', 'INTBOX'),
    1260: ('set4', 'BOLBOX'),
    1270: ('def4', 'INTBOX'),
    1280: ('set5', 'BOLBOX'),
    1290: ('def5', 'INTBOX'),
    1300: ('set6', 'BOLBOX'),
    1310: ('def6', 'INTBOX'),
    1360: ('breaks', 'INTBOX'),
    1370: ('broken', 'INTBOX'),
    1380: ('bornwith', 'INTBOX'),
    1390: ('secure', 'BOLBOX'),
    1400: ('unswitchable', 'BOLBOX'),
    1410: ('unremoveable', 'BOLBOX'),
    1420: ('undroppable', 'BOLBOX'),
    1430: ('nodeathdrop', 'BOLBOX'),
    1500: ('code', 'STRBOX'),
    1510: ('varspace', 'INTBOX'),
    1520: ('object1', 'INTBOX'),
    1530: ('object2', 'INTBOX'),
    1540: ('object3', 'INTBOX'),
}

# Retired/old properties — still need correct types to skip bytes properly.
# These use the same IDs but were superseded by newer fields.
OLD_PROPS = {
    150: ('_lookmsg', 'INTBOX'),
    160: ('_examinemsg', 'INTBOX'),
    170: ('_hideprops', 'BOLBOX'),
    180: ('_usemsg', 'INTBOX'),
    190: ('_roommsg', 'INTBOX'),
    200: ('_allmsg', 'INTBOX'),
    210: ('_alerter', 'BOLBOX'),
    220: ('_usealtermsg', 'INTBOX'),
    230: ('_roomaltermsg', 'INTBOX'),
    240: ('_allaltermsg', 'INTBOX'),
    250: ('_hitmsg', 'INTBOX'),
    260: ('_victimmsg', 'INTBOX'),
    300: ('_notoutside', 'BOLBOX'),
    310: ('_notinside', 'BOLBOX'),
    320: ('_allteamsin', 'BOLBOX'),
    430: ('_delay', 'INTBOX'),
    440: ('_mustwait', 'BOLBOX'),
    470: ('_intohand', 'BOLBOX'),
    480: ('_whichhand', 'INTBOX'),
    590: ('_connectsup', 'BOLBOX'),
    600: ('_connectsdown', 'BOLBOX'),
    610: ('_connectsleft', 'BOLBOX'),
    620: ('_connectsright', 'BOLBOX'),
    630: ('_connectsother', 'INTBOX'),
    640: ('_vehicle', 'INTBOX'),
    650: ('_firein', 'BOLBOX'),
    660: ('_drivespeed', 'INTBOX'),
    670: ('_entry', 'BOLBOX'),
    680: ('_driver', 'BOLBOX'),
    690: ('_window', 'BOLBOX'),
    700: ('_fireout', 'BOLBOX'),
    850: ('_vampire', 'BOLBOX'),
    900: ('_putsdown', 'INTBOX'),
    1020: ('_weartime', 'INTBOX'),
    1030: ('_defense', 'INTBOX'),
    1040: ('_absorb', 'INTBOX'),
    1050: ('_deflect', 'INTBOX'),
    1090: ('_winner', 'INTBOX'),
    1100: ('_carrybits', 'INTBOX'),
    1110: ('_wearbits', 'INTBOX'),
    1120: ('_holdbits', 'INTBOX'),
    1130: ('_theft', 'INTBOX'),
    1150: ('_pullcharges', 'BOLBOX'),
    1158: ('_ignoreauto', 'BOLBOX'),
    1160: ('_override', 'INTBOX'),
    1320: ('_autoactor', 'BOLBOX'),
    1330: ('_autotaken', 'BOLBOX'),
    1340: ('_autodelay', 'INTBOX'),
    1350: ('_autorepeat', 'INTBOX'),
    1440: ('_dropper', 'INTBOX'),
    1442: ('_fardropper', 'INTBOX'),
    1450: ('_destroyer', 'INTBOX'),
    1460: ('_creator', 'INTBOX'),
    1465: ('_multiple', 'INTBOX'),
    1470: ('_chains', 'BOLBOX'),
    1480: ('_chainobj', 'INTBOX'),
    1490: ('_makesmove', 'BOLBOX'),
}

# Merge: current properties take priority
ALL_PROPS = {}
ALL_PROPS.update(OLD_PROPS)
ALL_PROPS.update(OBJ_PROPS)


# ── binary reading helpers ──

def read_id(data, pos):
    """Read 2-byte big-endian unsigned ID."""
    return data[pos] * 256 + data[pos + 1], pos + 2


def read_sign_mag_short(data, pos):
    """Read 2-byte sign-magnitude short."""
    hi, lo = data[pos], data[pos + 1]
    neg = hi & 0x80
    val = (hi & 0x7F) * 256 + lo
    return (-val if neg else val), pos + 2


def read_sign_mag_long(data, pos):
    """Read 4-byte sign-magnitude long."""
    b0, b1, b2, b3 = data[pos], data[pos + 1], data[pos + 2], data[pos + 3]
    neg = b0 & 0x80
    val = (b0 & 0x7F) * (256 ** 3) + b1 * (256 ** 2) + b2 * 256 + b3
    return (-val if neg else val), pos + 4


def read_cstring(data, pos):
    """Read null-terminated string."""
    end = data.index(0, pos)
    return data[pos:end].decode('latin-1'), end + 1


def read_diag_record(data, pos, prop_table):
    """Read one diag record. Returns (fields_dict, new_pos) or (None, new_pos) on EOF/end-of-section."""
    if pos + 2 > len(data):
        return None, pos

    field_id, pos = read_id(data, pos)
    if field_id == ENDLIST_ID:
        return None, pos  # empty record / EOF
    if field_id == 32767:  # END_OF_SECTION
        return None, pos

    result = {}
    while field_id != ENDLIST_ID:
        if field_id == 32767:
            break

        prop = prop_table.get(field_id)
        if prop is None:
            print(f'  WARNING: unknown field ID {field_id} at offset {pos - 2}', file=sys.stderr)
            break

        name, typ = prop
        is_old = name.startswith('_')

        if typ == 'BOLBOX':
            if not is_old:
                result[name] = True
        elif typ in ('INTBOX', 'LINKPIC'):
            val, pos = read_sign_mag_short(data, pos)
            if not is_old:
                result[name] = val
        elif typ == 'LONGINT':
            val, pos = read_sign_mag_long(data, pos)
            if not is_old:
                result[name] = val
        elif typ in ('STRBOX', 'LSTRBOX', 'HIDSTR'):
            val, pos = read_cstring(data, pos)
            if not is_old:
                result[name] = val if val else None
        elif typ == 'ICONPIC':
            raw = list(data[pos:pos + BITMAP_ARRAY_SIZE])
            pos += BITMAP_ARRAY_SIZE
            if not is_old:
                result[name] = raw

        if pos + 2 > len(data):
            break
        field_id, pos = read_id(data, pos)

    return result, pos


# ── PNG writing ──

def write_png_gray(path, width, height, pixels):
    """Write 8-bit grayscale PNG."""
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


def bitmap_to_pixels(raw_bytes, width=32, height=32):
    """Convert raw XBM-style bitmap bytes to grayscale pixel list (0=black, 255=white)."""
    row_bytes = (width + 7) // 8
    pixels = []
    for y in range(height):
        for x in range(width):
            byte_idx = y * row_bytes + (x // 8)
            if byte_idx < len(raw_bytes):
                bit = (raw_bytes[byte_idx] >> (x % 8)) & 1
            else:
                bit = 0
            pixels.append(0 if bit else 255)
    return pixels


# ── obj file parsing ──

def parse_obj_file(path):
    """Parse a .obj file. Returns (header_lines, objects_list)."""
    with open(path, 'rb') as f:
        raw = f.read()

    # Split text header from binary data at the END_OF_MESSAGES sentinel
    sentinel = END_OF_MESSAGES.encode('latin-1') + b'\n'
    idx = raw.find(sentinel)
    if idx == -1:
        print(f'  ERROR: sentinel not found in {path}', file=sys.stderr)
        return [], []

    header_text = raw[:idx].decode('latin-1')
    binary_data = raw[idx + len(sentinel):]

    # Parse header for object count
    header_lines = header_text.split('\n')
    obj_count = 0
    for line in header_lines:
        m = re.search(r'object_set_size[:\s]*(\d+)', line)
        if m:
            obj_count = int(m.group(1))
            break

    # Parse binary object records
    objects = []
    pos = 0
    data = binary_data
    for i in range(obj_count):
        if pos >= len(data):
            break
        rec, pos = read_diag_record(data, pos, ALL_PROPS)
        if rec is None:
            break
        rec['_index'] = i
        objects.append(rec)

    return header_lines, objects


def export_obj(path, out_dir):
    """Parse one .obj file and write JSON + bitmap PNGs."""
    stem = os.path.splitext(os.path.basename(path))[0]
    header_lines, objects = parse_obj_file(path)

    bitmaps_dir = os.path.join(out_dir, 'bitmaps', stem)
    exported_bitmaps = 0

    json_objects = []
    for obj in objects:
        idx = obj.get('_index', 0)
        name = obj.get('name', f'obj_{idx}')

        # Extract and save bitmap/mask as PNGs, replace raw bytes with filename ref
        for field in ('bitmap', 'mask'):
            if field in obj:
                raw = obj[field]
                if any(b != 0 for b in raw):
                    png_name = f'{idx:03d}_{field}.png'
                    png_path = os.path.join(bitmaps_dir, png_name)
                    pixels = bitmap_to_pixels(raw)
                    write_png_gray(png_path, 32, 32, pixels)
                    exported_bitmaps += 1
                    obj[field] = png_name
                else:
                    del obj[field]  # all zeros, skip

        json_objects.append(obj)

    # Write JSON
    os.makedirs(out_dir, exist_ok=True)
    json_path = os.path.join(out_dir, f'{stem}.json')
    with open(json_path, 'w') as f:
        json.dump({
            'source': os.path.basename(path),
            'header': header_lines,
            'count': len(json_objects),
            'objects': json_objects,
        }, f, indent=2)

    return len(json_objects), exported_bitmaps


def main():
    if len(sys.argv) > 1:
        files = sys.argv[1:]
    else:
        obj_dir = os.path.join(REPO, 'lib', 'obj')
        files = sorted(os.path.join(obj_dir, f)
                       for f in os.listdir(obj_dir) if f.endswith('.obj'))

    total_objs, total_bmps = 0, 0
    for path in files:
        stem = os.path.splitext(os.path.basename(path))[0]
        n_objs, n_bmps = export_obj(path, OUT_DIR)
        print(f'  {stem + ".obj":<20} {n_objs:>3} objects, {n_bmps:>3} bitmaps')
        total_objs += n_objs
        total_bmps += n_bmps

    print(f'\n{total_objs} objects, {total_bmps} bitmaps written to {OUT_DIR}')


if __name__ == '__main__':
    main()
