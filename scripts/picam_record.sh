#!/usr/bin/env bash
set -euo pipefail
DURATION_MS="${1:-10000}"
OUTDIR="$HOME/iot-hub/media/picam/videos"
mkdir -p "$OUTDIR"
TS=$(date +%Y%m%d_%H%M%S)
OUT="$OUTDIR/$TS.h264"
rpicam-vid -t "$DURATION_MS" -o "$OUT"
echo "$OUT"
