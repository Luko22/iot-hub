<!doctype html>
<html>
<head>
  <link rel="icon" href="/robot.ico">
  <meta charset="utf-8" />
  <title>IoT Hub</title>
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <style>
    body { font-family: system-ui, -apple-system, Segoe UI, Roboto, Arial; margin: 40px; }
    .card { max-width: 720px; padding: 18px 20px; border: 1px solid #ddd; border-radius: 12px; margin-bottom: 14px; }
    a { text-decoration: none; }
  </style>
</head>
<body>
  <h1>IoT Hub</h1>

 <div class="card">
    <h2>Raspberry Pi Camera</h2>
    <p>Take photos and videos using the Raspberry Pi camera module.</p>
    <p><a href="http://192.168.230.93:1984/stream.html?src=picam&mode=webrtc">Start Pi camera Stream</a></p>
  </div>

  <div class="card">
    <h2>Grafana Dashboards</h2>
    <p>Geospatial and Telemetry data from PostgreSQL.</p>
    <p><a href="http://192.168.230.93:3000/public-dashboards/a2d0d57861b64c0eb79746b65ecd38f9" target="_blank">Open Grafana Dashboard</a></p>
  </div>

  <div class="card">
    <h2>API</h2>
    <p><a href="/api/devices" target="_blank">/api/devices</a></p>
  </div>
</body>
</html>

