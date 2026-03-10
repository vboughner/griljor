# Pipeline

This directory contains the asset extraction scripts and their output.

## Scripts

| Script | Purpose |
|--------|---------|
| `xbm_to_png.py` | Convert raw 1-bit XBM bitmaps → 8-bit grayscale PNGs |
| `parse_objs.py` | Parse binary `.obj` files → `data/objects/*.json` |
| `parse_maps.py` | Parse binary `.map` files → `data/maps/*.json` |
| `run_pipeline.sh` | Run all three scripts in order |

Requirements: Python 3 + Pillow (`pip install Pillow`).

## `out/` is committed intentionally

`pipeline/out/` contains all pipeline-generated assets and **is tracked by git**.
Do not delete it assuming it is a throwaway build artifact.

The browser client (`client/`) serves these files directly via Vite's `publicDir`
pointing at `pipeline/out/`. Without them, the client cannot run.

Re-run the pipeline only if you change a parser script or discover a parsing bug:

```sh
cd pipeline
./run_pipeline.sh
```
