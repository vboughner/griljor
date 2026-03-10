#!/usr/bin/env python3
"""
parse_maps.py — Parse Griljor .map files into JSON.

Two on-disk formats exist:

  OLD (v1.0) raw-struct format (25 of 26 maps):
    1. MapInfo struct (2126 bytes, big-endian)
    2. N × RoomInfo struct (944 bytes each) + M × RecordedObj (18 bytes each)
       interleaved per room: each RoomInfo is immediately followed by its
       numobjs RecordedObj records.

  NEW (v2.0) diag format (standard.map only):
    1. MapInfo diag record (ENDLIST)
    2. N × RoomInfo diag records (each ENDLIST), then END_OF_SECTION (0x7FFF)
    3. M × MapObject instance diag records (each ENDLIST), then END_OF_SECTION

  Format detection: if first 2 bytes are 0x00 0x64 (diag field ID 100 = map name)
  then it's the new format; otherwise it's the old raw-struct format.

Struct sizes (Sun3/68k platform, big-endian, natural alignment):
  MapInfo:     2126 bytes
  RoomInfo:     944 bytes  (943 data + 1 trailing alignment pad)
  RecordedObj:   18 bytes  (3 chars + 1 pad + 7 shorts)

Field offsets in RoomInfo:
  0   name[80]
  80  floor (uchar)
  81  team (uchar)
  82  exit[4] (4 × big-endian short)
  90  numobjs (uchar)
  91  spot[ROOM_WIDTH][ROOM_HEIGHT][ROOM_DEPTH] (20×20×2 uchars = 800 bytes)
  891 appearance (char)
  892 dark (char)
  893 pad[50]  +  1 trailing byte  = 944 total

Field offsets in RecordedObj:
  0  x (char)
  1  y (char)
  2  objtype (uchar)
  3  (alignment pad)
  4  detail (big-endian short)
  6  infox  (big-endian short)
  8  infoy  (big-endian short)
  10 zinger (big-endian short)
  12 extra[3] (3 × big-endian short)

Property IDs for the new diag format derived from src/objprops.c.
"""

import os, sys, json, struct as _struct

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_DIR = os.path.join(REPO, 'pipeline', 'out', 'data', 'maps')

# ── old-format constants ──

OLD_MAPINFO_SIZE  = 2126
OLD_ROOMINFO_SIZE = 944
OLD_RECOBJ_SIZE   = 18

ROOM_WIDTH  = 20
ROOM_HEIGHT = 20
ROOM_DEPTH  = 2

# ── diag-format constants ──

ENDLIST_ID    = 0
END_SECTION_ID = 32767  # 0x7FFF
BITMAP_ARRAY_SIZE = 128


# ── diag property tables (from src/objprops.c) ──

MAP_PROPS = {
    100: ('name',           'STRBOX'),
    110: ('team_name_1',    'STRBOX'),
    120: ('team_name_2',    'STRBOX'),
    130: ('team_name_3',    'STRBOX'),
    140: ('team_name_4',    'STRBOX'),
    145: ('neutral_name',   'STRBOX'),
    150: ('teams_supported','INTBOX'),
    155: ('neutrals_allowed','BOLBOX'),
    160: ('objfilename',    'STRBOX'),
    170: ('execute_file',   'STRBOX'),
    180: ('startup_file',   'STRBOX'),
    190: ('placement_file', 'STRBOX'),
}

ROOM_PROPS = {
    100: ('name',          'STRBOX'),
    110: ('dark',          'BOLBOX'),
    120: ('floor',         'INTBOX'),
    130: ('team',          'INTBOX'),
    140: ('cycles',        'BOLBOX'),
    150: ('limited_sight', 'BOLBOX'),
    160: ('objects_appear','BOLBOX'),
    170: ('people_appear', 'BOLBOX'),
    180: ('object_floor',  'INTBOX'),
    190: ('people_floor',  'INTBOX'),
    200: ('exit_north',    'INTBOX'),
    210: ('exit_east',     'INTBOX'),
    220: ('exit_south',    'INTBOX'),
    230: ('exit_west',     'INTBOX'),
}

MAPOBJ_PROPS = {
    100: ('type',         'INTBOX'),
    110: ('id',           'LONGINT'),
    111: ('contained_id', 'LONGINT'),
    112: ('container_id', 'LONGINT'),
    113: ('lsibling_id',  'LONGINT'),
    114: ('rsibling_id',  'LONGINT'),
    120: ('detail',       'INTBOX'),
    130: ('infox',        'INTBOX'),
    140: ('infoy',        'INTBOX'),
    150: ('zinger',       'INTBOX'),
    160: ('extra1',       'INTBOX'),
    170: ('extra2',       'INTBOX'),
    180: ('extra3',       'INTBOX'),
    200: ('room',         'INTBOX'),
    210: ('locx',         'INTBOX'),
    220: ('locy',         'INTBOX'),
    300: ('info_follows', 'BOLBOX'),
}


# ── diag binary reading helpers ──

def _read_id(data, pos):
    if pos + 2 > len(data):
        return -1, pos
    return data[pos] * 256 + data[pos + 1], pos + 2


def _read_sign_mag_short(data, pos):
    hi, lo = data[pos], data[pos + 1]
    neg = hi & 0x80
    val = (hi & 0x7F) * 256 + lo
    return (-val if neg else val), pos + 2


def _read_sign_mag_long(data, pos):
    b0, b1, b2, b3 = data[pos], data[pos+1], data[pos+2], data[pos+3]
    neg = b0 & 0x80
    val = (b0 & 0x7F) * (256**3) + b1 * (256**2) + b2 * 256 + b3
    return (-val if neg else val), pos + 4


def _read_cstring(data, pos):
    end = data.index(0, pos)
    return data[pos:end].decode('latin-1'), end + 1


def _read_diag_record(data, pos, prop_table):
    """Read one diag record. Returns (dict, new_pos, end_of_section)."""
    if pos + 2 > len(data):
        return None, pos, False

    field_id, pos = _read_id(data, pos)
    if field_id == ENDLIST_ID:
        return None, pos, False
    if field_id == END_SECTION_ID:
        return None, pos, True

    result = {}
    while field_id != ENDLIST_ID:
        if field_id == END_SECTION_ID:
            break
        prop = prop_table.get(field_id)
        if prop is None:
            print(f'  WARNING: unknown field ID {field_id} at offset {pos-2}',
                  file=sys.stderr)
            break
        name, typ = prop
        if typ == 'BOLBOX':
            result[name] = True
        elif typ in ('INTBOX', 'LINKPIC'):
            val, pos = _read_sign_mag_short(data, pos)
            result[name] = val
        elif typ == 'LONGINT':
            val, pos = _read_sign_mag_long(data, pos)
            result[name] = val
        elif typ in ('STRBOX', 'LSTRBOX', 'HIDSTR'):
            val, pos = _read_cstring(data, pos)
            result[name] = val if val else None
        elif typ == 'ICONPIC':
            result[name] = list(data[pos:pos + BITMAP_ARRAY_SIZE])
            pos += BITMAP_ARRAY_SIZE
        if pos + 2 > len(data):
            break
        field_id, pos = _read_id(data, pos)

    return result, pos, False


def _read_diag_section(data, pos, prop_table):
    """Read diag records until end-of-section or EOF."""
    records = []
    while pos < len(data):
        rec, pos, eos = _read_diag_record(data, pos, prop_table)
        if rec is None:
            break
        records.append(rec)
    return records, pos


# ── old-format raw-struct parsing ──

def _cstr(data, offset, length):
    chunk = data[offset:offset + length]
    return chunk.split(b'\x00')[0].decode('latin-1', errors='replace') or None


def _parse_old_mapinfo(data):
    """Parse the 2126-byte old-format MapInfo struct."""
    team_names = [_cstr(data, 80 + i * 80, 80) for i in range(4)]
    teams_supported = _struct.unpack_from('>h', data, 400)[0]
    rooms_count     = _struct.unpack_from('>i', data, 582)[0]
    return {
        'name':             _cstr(data, 0, 80),
        'team_name_1':      team_names[0],
        'team_name_2':      team_names[1],
        'team_name_3':      team_names[2],
        'team_name_4':      team_names[3],
        'teams_supported':  teams_supported,
        'objfilename':      _cstr(data, 402, 180),
        'rooms_count':      rooms_count,
        'execute_file':     _cstr(data, 586, 180),
        'startup_file':     _cstr(data, 766, 180),
        'placement_file':   _cstr(data, 946, 180),
        'neutrals_allowed': bool(data[1126]),
    }, rooms_count


def _parse_old_roominfo(data, offset):
    """Parse one 944-byte RoomInfo struct. Returns (room_dict, numobjs)."""
    exits = list(_struct.unpack_from('>4h', data, offset + 82))
    numobjs = data[offset + 90]

    # spot[ROOM_WIDTH][ROOM_HEIGHT][ROOM_DEPTH]: tile object IDs
    spot_raw = data[offset + 91 : offset + 91 + ROOM_WIDTH * ROOM_HEIGHT * ROOM_DEPTH]
    # Reformat as list of rows: spot[x][y] = [layer0, layer1]
    spot = []
    for x in range(ROOM_WIDTH):
        col = []
        for y in range(ROOM_HEIGHT):
            base = (x * ROOM_HEIGHT + y) * ROOM_DEPTH
            col.append(list(spot_raw[base:base + ROOM_DEPTH]))
        spot.append(col)

    return {
        'name':       _cstr(data, offset, 80),
        'floor':      data[offset + 80],
        'team':       data[offset + 81],
        'exit_north': exits[0],
        'exit_east':  exits[1],
        'exit_south': exits[2],
        'exit_west':  exits[3],
        'appearance': data[offset + 891],
        'dark':       data[offset + 892],
        'spot':       spot,
    }, numobjs


def _parse_old_recobj(data, offset):
    """Parse one 18-byte RecordedObj struct."""
    x       = _struct.unpack_from('>b', data, offset)[0]
    y       = _struct.unpack_from('>b', data, offset + 1)[0]
    objtype = data[offset + 2]
    # offset+3 is padding
    detail  = _struct.unpack_from('>h', data, offset +  4)[0]
    infox   = _struct.unpack_from('>h', data, offset +  6)[0]
    infoy   = _struct.unpack_from('>h', data, offset +  8)[0]
    zinger  = _struct.unpack_from('>h', data, offset + 10)[0]
    extra   = list(_struct.unpack_from('>3h', data, offset + 12))
    return {
        'x': x, 'y': y, 'type': objtype,
        'detail': detail, 'infox': infox, 'infoy': infoy,
        'zinger': zinger, 'extra': extra,
    }


def _parse_old_format(data):
    """Parse a v1.0 raw-struct .map file."""
    map_info, rooms_count = _parse_old_mapinfo(data)

    rooms = []
    pos = OLD_MAPINFO_SIZE
    for _ in range(rooms_count):
        room, numobjs = _parse_old_roominfo(data, pos)
        pos += OLD_ROOMINFO_SIZE
        room['recorded_objects'] = []
        for _ in range(numobjs):
            room['recorded_objects'].append(_parse_old_recobj(data, pos))
            pos += OLD_RECOBJ_SIZE
        rooms.append(room)

    return map_info, rooms, []   # no MapObject list in old format


# ── top-level parsing ──

def parse_map_file(path):
    """Parse a .map file. Returns dict with map_info, rooms, and objects."""
    with open(path, 'rb') as f:
        data = f.read()

    # Detect format: new diag format starts with field ID 100 (0x00 0x64)
    is_diag = (len(data) >= 2 and data[0] == 0x00 and data[1] == 0x64)

    if is_diag:
        pos = 0
        map_info, pos, _ = _read_diag_record(data, pos, MAP_PROPS)
        if map_info is None:
            map_info = {}
        rooms, pos = _read_diag_section(data, pos, ROOM_PROPS)
        objects, _ = _read_diag_section(data, pos, MAPOBJ_PROPS)
    else:
        map_info, rooms, objects = _parse_old_format(data)

    return {
        'source':       os.path.basename(path),
        'format':       'diag' if is_diag else 'struct',
        'map':          map_info,
        'rooms':        rooms,
        'room_count':   len(rooms),
        'objects':      objects,
        'object_count': len(objects),
    }


def export_map(path, out_dir):
    """Parse one .map file, write JSON, return (format, room_count, object_count)."""
    stem = os.path.splitext(os.path.basename(path))[0]
    result = parse_map_file(path)

    os.makedirs(out_dir, exist_ok=True)
    json_path = os.path.join(out_dir, f'{stem}.json')
    with open(json_path, 'w') as f:
        json.dump(result, f, indent=2)

    return result['format'], result['room_count'], result['object_count']


def main():
    if len(sys.argv) > 1:
        files = sys.argv[1:]
    else:
        map_dir = os.path.join(REPO, 'lib', 'map')
        files = sorted(os.path.join(map_dir, f)
                       for f in os.listdir(map_dir) if f.endswith('.map'))

    total_rooms, total_objs = 0, 0
    for path in files:
        stem = os.path.splitext(os.path.basename(path))[0]
        fmt, n_rooms, n_objs = export_map(path, OUT_DIR)
        fmt_tag = 'diag  ' if fmt == 'diag' else 'struct'
        print(f'  {stem + ".map":<20} [{fmt_tag}] {n_rooms:>3} rooms, {n_objs:>4} objects')
        total_rooms += n_rooms
        total_objs += n_objs

    print(f'\n{total_rooms} rooms, {total_objs} objects written to {OUT_DIR}')


if __name__ == '__main__':
    main()
