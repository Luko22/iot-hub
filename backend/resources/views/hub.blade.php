<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <title>IoT Hub</title>
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <style>
    body { font-family: system-ui, -apple-system, Segoe UI, Roboto, Arial; margin: 40px; }
    .card { max-width: 820px; padding: 18px 20px; border: 1px solid #ddd; border-radius: 12px; margin-bottom: 14px; }
    a { text-decoration: none; }
    .muted { color: #555; }
  </style>
</head>
<body>
  <h1>IoT Hub</h1>
  <p class="muted">Local-first dashboard (Raspberry Pi).</p>

  <div class="card">
    <h2>Geospatial Telemetry</h2>
    <p>3D globe visualization + latest telemetry.</p>
    <p><a href="/globe">Open Globe</a></p>
  </div>

  <div class="card">
    <h2>Grafana Dashboards</h2>
    <p>Temperature vs time and humidity vs time.</p>
    <p><a href="http://localhost:3000" target="_blank" rel="noopener">Open Grafana</a></p>
  </div>

  <div class="card">
    <h2>API</h2>
    <p><a href="/api/devices" target="_blank" rel="noopener">/api/devices</a></p>
    <p><a href="/api/devices/espKleveNeo7M/latest" target="_blank" rel="noopener">/api/devices/{device}/latest</a></p>
  </div>
</body>
</html>
