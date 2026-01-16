#!/usr/bin/env bash
set -euo pipefail
OUTDIR="$HOME/iot-hub/media/picam/photos"
mkdir -p "$OUTDIR"
TS=$(date +%Y%m%d_%H%M%S)
OUT="$OUTDIR/$TS.jpg"
rpicam-still -o "$OUT"
echo "$OUT"
