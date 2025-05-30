const express = require('express');
const fetch = require('node-fetch');
const pool = require('./db'); // PostgreSQL connection

const app = express();
const port = 3000;

app.use(express.json()); // Parse incoming JSON payloads

//SENSORID = esp32-868600 
//Index = 56391

/**
 * Sends data to Sensor.Community via HTTP POST.
 * @param {string} sensorId - The internal ID of the sensor (custom).
 * @param {number} xPin - The X-Pin value (defines sensor type: 1 = PM, 11 = temp, etc.).
 * @param {string} xSensor - Unique Sensor.Community sensor identifier (e.g., esp8266-011111).
 * @param {Array} values - Array of { value_type, value } objects.
 */
async function sendToSensorCommunity(sensorId, xPin, xSensor, values) {
  const url = 'https://api.sensor.community/v1/push-sensor-data/';

  const body = values;

  const response = await fetch(url, {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'X-Pin': xPin.toString(),
      'X-Sensor': xSensor
    },
    body: JSON.stringify(body)
  });

  if (!response.ok) {
    const text = await response.text();
    throw new Error(`Sensor.Community error: ${response.status} ${text}`);
  }
}

/**
 * Returns the full Sensor.Community-compatible sensor ID.
 * @param {string} sensorId - Custom internal ID
 */
function getXSensor(sensorId) {
  return `esp8266-${sensorId}`;
}

/**
 * Prepares and sends sensor data to Sensor.Community depending on available fields.
 * Splits data per sensor type using different X-Pins.
 * @param {string} sensorId 
 * @param {object} data - Payload containing temperature, pressure, airquality, sound
 */
async function handleSensorCommunityUpload(sensorId, data) {
  const xSensor = getXSensor(sensorId);

  // Air quality (dust)
  if (data.airquality !== undefined) {
    await sendToSensorCommunity(
      sensorId,
      1, // PM data
      xSensor,
      [
        { value_type: "P1", value: data.airquality.toString() } // PM10
      ]
    );
  }

  // Temperature and pressure (BMP280)
  if (data.temperature !== undefined || data.pressure !== undefined) {
    const values = [];
    if (data.temperature !== undefined) {
      values.push({ value_type: "temperature", value: data.temperature.toString() });
    }
    if (data.pressure !== undefined) {
      values.push({ value_type: "pressure", value: data.pressure.toString() });
    }

    await sendToSensorCommunity(
      sensorId,
      11, // environmental sensor
      xSensor,
      values
    );
  }

  // Sound (SPH0645)
  if (data.sound !== undefined) {
    await sendToSensorCommunity(
      sensorId,
      13, // not official, but accepted
      xSensor,
      [
        { value_type: "noise", value: data.sound.toString() }
      ]
    );
  }
}

/**
 * POST /data endpoint
 * Accepts a JSON payload with sensor values and:
 *  - stores them in PostgreSQL
 *  - sends them to Sensor.Community
 */
app.post('/data', async (req, res) => {
  const { sensorId, temperature, pressure, airquality, sound } = req.body;

  if (!sensorId) {
    return res.status(400).send('sensorId is required');
  }

  // Set undefined fields to null for database compatibility
  const tempVal = temperature !== undefined ? temperature : null;
  const presVal = pressure !== undefined ? pressure : null;
  const airqVal = airquality !== undefined ? airquality : null;
  const soundVal = sound !== undefined ? sound : null;

  try {
    // Store in local PostgreSQL database
    await pool.query(
      `INSERT INTO sensors (sensor_id, temperature, pressure, airquality, sound) 
       VALUES ($1, $2, $3, $4, $5)`,
      [sensorId, tempVal, presVal, airqVal, soundVal]
    );

    // Forward data to Sensor.Community
    await handleSensorCommunityUpload(sensorId, req.body);

    res.status(201).send('Data stored and sent');
  } catch (err) {
    console.error(err);
    res.status(500).send('Server error');
  }
});

// Start the Express API server
app.listen(port, () => {
  console.log(`API running at http://localhost:${port}`);
});
