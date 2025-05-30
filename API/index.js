const express = require('express');
// Dynamically import node-fetch for ESM compatibility
let fetch;
(async () => {
  fetch = (await import('node-fetch')).default;
})();
const pool = require('./db'); // PostgreSQL connection

const app = express();
const port = 3000;

app.use(express.json());

/**
 * Send sensor data to the Sensor Community API.
 * @param {Object} data - The sensor data to send.
 * @param {string} sensorId - The sensor ID for Sensor Community.
 */
async function sendToSensorsCommunity(data, sensorId) {
  // Wait for fetch to be loaded if not already
  if (!fetch) {
    fetch = (await import('node-fetch')).default;
  }

  const body = {
    software_version: "nodejs_api_1.0",
    sensordatavalues: []
  };

  // Build the payload according to the sensor type
  switch(sensorId) {
    case "011111": // SPH0645 (Sound sensor)
      if (data.sound !== undefined) {
        body.sensordatavalues.push({ value_type: "sound", value: data.sound.toString() });
      }
      break;
    case "022222": // BMP280 (Temperature/Pressure/Humidity sensor)
      if (data.temperature !== undefined) {
        body.sensordatavalues.push({ value_type: "temperature", value: data.temperature.toString() });
      }
      if (data.pressure !== undefined) {
        body.sensordatavalues.push({ value_type: "pressure", value: data.pressure.toString() });
      }
      break;
    case "033333": // Dust sensor
      if (data.airquality !== undefined) {
        body.sensordatavalues.push({ value_type: "P1", value: data.airquality.toString() }); // P1 = PM10 by convention
      }
      break;
    default:
      throw new Error(`Unknown SensorID: ${sensorId}`);
  }

  const url = `https://data.sensor.community/airrohr/v1/push-sensor-data/?sensor=${sensorId}`;

  const response = await fetch(url, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(body),
  });

  if (!response.ok) {
    const text = await response.text();
    throw new Error(`Sensor Community error: ${response.status} ${text}`);
  }
}

// New route to receive an array of measurements from the ESP receiver
app.post('/espdata', async (req, res) => {
  const dataArray = req.body;
  // Log received data for debugging
  console.log('Data received from ESP:', dataArray);

  // Check that the request body is a non-empty JSON array
  if (!Array.isArray(dataArray) || dataArray.length === 0) {
    return res.status(400).send('Body must be a non-empty JSON array');
  }

  // Fixed sensor IDs for each type
  const ENV_SENSOR_ID = "022222"; // temp, hum, pressure
  const SOUND_SENSOR_ID = "011111"; // sound
  const DUST_SENSOR_ID = "033333"; // dust/airquality

  try {
    for (const data of dataArray) {
      // Insert and send for temp/hum/pressure
      if (
        data.temp !== undefined ||
        data.hum !== undefined ||
        data.press !== undefined
      ) {
        const tempVal = data.temp !== undefined ? data.temp : null;
        const presVal = data.press !== undefined ? data.press : null;
        const humVal = data.hum !== undefined ? data.hum : null;

        // Insert environmental data into the database
        await pool.query(
          `INSERT INTO sensors (sensor_id, temperature, pressure, humidity) 
           VALUES ($1, $2, $3, $4)`,
          [ENV_SENSOR_ID, tempVal, presVal, humVal]
        );

        // Send environmental data to Sensor Community
        await sendToSensorsCommunity(
          { temperature: tempVal, pressure: presVal, humidity: humVal },
          ENV_SENSOR_ID
        );
      }

      // Insert and send for sound
      if (data.sound !== undefined) {
        const soundVal = data.sound;
        // Insert sound data into the database
        await pool.query(
          `INSERT INTO sensors (sensor_id, sound) VALUES ($1, $2)`,
          [SOUND_SENSOR_ID, soundVal]
        );

        // Send sound data to Sensor Community
        await sendToSensorsCommunity(
          { sound: soundVal },
          SOUND_SENSOR_ID
        );
      }

      // Insert and send for dust/airquality
      if (data.airquality !== undefined) {
        const airqVal = data.airquality;
        // Insert air quality data into the database
        await pool.query(
          `INSERT INTO sensors (sensor_id, airquality) VALUES ($1, $2)`,
          [DUST_SENSOR_ID, airqVal]
        );

        // Send air quality data to Sensor Community
        await sendToSensorsCommunity(
          { airquality: airqVal },
          DUST_SENSOR_ID
        );
      }
    }
    res.status(201).send('ESP data saved');
  } catch (err) {
    console.error(err);
    res.status(500).send('Server error');
  }
});

app.listen(port, () => {
  console.log(`API running at http://localhost:${port}`);
});
