#!/usr/bin/env bash
# run_pipeline.sh — Run the full Griljor asset pipeline.
#
# Converts all raw game assets to web-friendly formats:
#   bitmaps/, bit/, facebits/  →  pipeline/out/sprites/{dir}/*.png
#   lib/obj/*.obj              →  pipeline/out/data/objects/*.json
#                                 pipeline/out/data/objects/bitmaps/**/*.png
#   lib/map/*.map              →  pipeline/out/data/maps/*.json

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(dirname "$SCRIPT_DIR")"

cd "$REPO"

echo "=== Griljor Asset Pipeline ==="
echo

echo "--- Step 1: XBM bitmaps → PNG ---"
python3 pipeline/xbm_to_png.py
echo

echo "--- Step 2: Object definitions (.obj) → JSON + PNG ---"
python3 pipeline/parse_objs.py
echo

echo "--- Step 3: Map files (.map) → JSON ---"
python3 pipeline/parse_maps.py
echo

echo "=== Pipeline complete ==="
echo "Output is in pipeline/out/"
