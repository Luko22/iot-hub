# Camera HTTP Service

A lightweight Flask service that exposes HTTP endpoints to control the Raspberry Pi camera from your IoT Hub page. It starts the `rpicam-vid` listener for go2rtc, and triggers snapshots/recordings using the existing scripts.

## Endpoints
- `POST /camera/start` — starts `rpicam-vid -t 0 --inline --listen --nopreview -o tcp://0.0.0.0:8556`
- `POST /camera/stop` — stops the running `rpicam-vid`
- `POST /camera/snapshot` — runs `scripts/picam_snapshot.sh`, returns JSON with `url` under `/media`
- `POST /camera/record?duration_ms=10000` — runs `scripts/picam_record.sh`, returns JSON with `url`
- `GET /healthz` — simple health endpoint

## Requirements
- Python 3.9+
- Raspberry Pi `rpicam-apps` installed (`rpicam-vid`, `rpicam-still`)
- This repo at `~/iot-hub`

Install dependencies:

```bash
python3 -m venv ~/iot-hub/.venv
source ~/iot-hub/.venv/bin/activate
pip install -r ~/iot-hub/scripts/requirements.txt
```

## Run the service

```bash
python ~/iot-hub/scripts/camera_http_service.py
```

It listens on port `5001` (configurable with `CAMERA_SERVICE_PORT`). Ensure your IoT Hub page is served from the same host so it can call this service. The page will redirect to the go2rtc player at `http://<host>:1984/stream.html?src=picam&mode=webrtc`.

### Optional: run as a systemd service
Create `/etc/systemd/system/camera-http.service`:

```
[Unit]
Description=Camera HTTP Service
After=network.target

[Service]
ExecStart=/home/pi/iot-hub/.venv/bin/python /home/pi/iot-hub/scripts/camera_http_service.py
WorkingDirectory=/home/pi/iot-hub
Restart=on-failure
User=pi
Environment=CAMERA_SERVICE_PORT=5001

[Install]
WantedBy=multi-user.target
```

Then:

```bash
sudo systemctl daemon-reload
sudo systemctl enable --now camera-http.service
```

## Media outputs
The service returns URLs under `/media/picam/...`. Nginx already mounts `./media:/var/media`, so files placed in `~/iot-hub/media/picam/...` are reachable via your web server.
