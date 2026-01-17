#!/usr/bin/env python3
import os
import json
import signal
import subprocess
from flask import Flask, request, jsonify, make_response

# Configuration
HOST = os.environ.get('CAMERA_SERVICE_HOST', '0.0.0.0')
PORT = int(os.environ.get('CAMERA_SERVICE_PORT', '5001'))
PID_FILE = os.environ.get('RPICAM_VID_PID', '/tmp/rpicam_vid.pid')
STREAM_CMD = os.environ.get(
    'RPICAM_VID_CMD',
    'rpicam-vid -t 0 --inline --listen --nopreview -o tcp://0.0.0.0:8556'
)
# Paths: adjust MEDIA_BASE if needed
WORKSPACE = os.path.expanduser('~/iot-hub')
MEDIA_BASE = os.environ.get('MEDIA_BASE', os.path.join(WORKSPACE, 'media', 'picam'))
PHOTOS_DIR = os.path.join(MEDIA_BASE, 'photos')
VIDEOS_DIR = os.path.join(MEDIA_BASE, 'videos')

SNAPSHOT_SCRIPT = os.path.join(WORKSPACE, 'scripts', 'picam_snapshot.sh')
RECORD_SCRIPT = os.path.join(WORKSPACE, 'scripts', 'picam_record.sh')

app = Flask(__name__)


def cors_response(response):
    response.headers['Access-Control-Allow-Origin'] = '*'
    response.headers['Access-Control-Allow-Methods'] = 'GET,POST,OPTIONS'
    response.headers['Access-Control-Allow-Headers'] = 'Content-Type'
    return response


@app.route('/camera/start', methods=['POST', 'OPTIONS'])
def camera_start():
    if request.method == 'OPTIONS':
        return cors_response(make_response('', 200))
    # Ensure single instance
    if os.path.exists(PID_FILE):
        try:
            with open(PID_FILE, 'r') as f:
                pid = int(f.read().strip())
            os.kill(pid, 0)  # Check if process exists
            return cors_response(jsonify({'status': 'already_running', 'pid': pid}))
        except Exception:
            # Stale pid file
            try:
                os.remove(PID_FILE)
            except Exception:
                pass
    # Start process in background
    try:
        proc = subprocess.Popen(
            STREAM_CMD,
            shell=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            preexec_fn=os.setpgrp  # detach from parent
        )
        with open(PID_FILE, 'w') as f:
            f.write(str(proc.pid))
        return cors_response(jsonify({'status': 'started', 'pid': proc.pid}))
    except Exception as e:
        return cors_response(make_response(jsonify({'error': str(e)}), 500))


@app.route('/camera/stop', methods=['POST', 'OPTIONS'])
def camera_stop():
    if request.method == 'OPTIONS':
        return cors_response(make_response('', 200))
    if not os.path.exists(PID_FILE):
        return cors_response(jsonify({'status': 'not_running'}))
    try:
        with open(PID_FILE, 'r') as f:
            pid = int(f.read().strip())
        # Kill process group to ensure listener stops
        os.killpg(pid, signal.SIGTERM)
        try:
            os.remove(PID_FILE)
        except Exception:
            pass
        return cors_response(jsonify({'status': 'stopped'}))
    except Exception as e:
        return cors_response(make_response(jsonify({'error': str(e)}), 500))


@app.route('/camera/snapshot', methods=['POST', 'OPTIONS'])
def camera_snapshot():
    if request.method == 'OPTIONS':
        return cors_response(make_response('', 200))
    try:
        os.makedirs(PHOTOS_DIR, exist_ok=True)
        # Run snapshot script; it should echo the output path
        proc = subprocess.run([SNAPSHOT_SCRIPT], capture_output=True, text=True, check=False)
        if proc.returncode != 0:
            return cors_response(make_response(jsonify({'error': proc.stderr.strip() or 'snapshot failed'}), 500))
        output_path = proc.stdout.strip().splitlines()[-1] if proc.stdout else ''
        # Derive URL under /media
        url = derive_media_url(output_path)
        return cors_response(jsonify({'status': 'ok', 'path': output_path, 'url': url}))
    except Exception as e:
        return cors_response(make_response(jsonify({'error': str(e)}), 500))


@app.route('/camera/record', methods=['POST', 'OPTIONS'])
def camera_record():
    if request.method == 'OPTIONS':
        return cors_response(make_response('', 200))
    try:
        os.makedirs(VIDEOS_DIR, exist_ok=True)
        duration_ms = request.args.get('duration_ms', '10000')
        proc = subprocess.run([RECORD_SCRIPT, duration_ms], capture_output=True, text=True, check=False)
        if proc.returncode != 0:
            return cors_response(make_response(jsonify({'error': proc.stderr.strip() or 'record failed'}), 500))
        output_path = proc.stdout.strip().splitlines()[-1] if proc.stdout else ''
        url = derive_media_url(output_path)
        return cors_response(jsonify({'status': 'ok', 'path': output_path, 'url': url}))
    except Exception as e:
        return cors_response(make_response(jsonify({'error': str(e)}), 500))


@app.route('/healthz', methods=['GET'])
def healthz():
    return cors_response(jsonify({'status': 'ok'}))


@app.after_request
def add_cors_headers(response):
    return cors_response(response)


def derive_media_url(path):
    try:
        # Expect path under ~/iot-hub/media/picam/... -> /media/picam/...
        rel = os.path.relpath(path, os.path.join(WORKSPACE, 'media'))
        return f"/media/{rel.replace(os.sep, '/')}"
    except Exception:
        return ''


if __name__ == '__main__':
    app.run(host=HOST, port=PORT)
