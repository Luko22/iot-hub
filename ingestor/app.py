import json
import psycopg2
import paho.mqtt.client as mqtt

MQTT_BROKER = "mosquitto"
MQTT_TOPIC = "iot/+/telemetry"

DB_CONFIG = {
    "dbname": "iot",
    "user": "iotuser",
    "password": "iotpass",
    "host": "postgres",
}

conn = psycopg2.connect(**DB_CONFIG)
cur = conn.cursor()

def on_connect(client, userdata, flags, rc):
    print("MQTT connected with code", rc)
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    payload = json.loads(msg.payload.decode())

    cur.execute(
        """
        INSERT INTO telemetry (device_id, latitude, longitude, temperature, humidity)
        VALUES (%s, %s, %s, %s, %s)
        """,
        (
            payload["device_id"],
            payload["lat"],
            payload["lon"],
            payload["temperature"],
            payload["humidity"],
        ),
    )
    conn.commit()
    print("Inserted:", payload)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_BROKER, 1883)
client.loop_forever()
