const express = require('express');
const fetch = require('node-fetch');
const pool = require('./db'); // ta connexion PostgreSQL

const app = express();
const port = 3000;

app.use(express.json());

async function sendToSensorsCommunity(data, sensorId) {
  const body = {
    software_version: "nodejs_api_1.0",
    sensordatavalues: []
  };

  switch(sensorId) {
    case "011111": // SPH0645
      if (data.sound !== undefined) {
        body.sensordatavalues.push({ value_type: "sound", value: data.sound.toString() });
      }
      break;
    case "022222": // BMP280
      if (data.temperature !== undefined) {
        body.sensordatavalues.push({ value_type: "temperature", value: data.temperature.toString() });
      }
      if (data.pressure !== undefined) {
        body.sensordatavalues.push({ value_type: "pressure", value: data.pressure.toString() });
      }
      break;
    case "033333": // Dust sensor
      if (data.airquality !== undefined) {
        body.sensordatavalues.push({ value_type: "P1", value: data.airquality.toString() }); // P1 = PM10 par convention
      }
      break;
    default:
      throw new Error(`SensorID inconnu: ${sensorId}`);
  }

  const url = `https://data.sensor.community/airrohr/v1/push-sensor-data/?sensor=${sensorId}`;

  const response = await fetch(url, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(body),
  });

  if (!response.ok) {
    const text = await response.text();
    throw new Error(`Erreur sensors.community: ${response.status} ${text}`);
  }
}

app.post('/data', async (req, res) => {
  const { sensorId, temperature, pressure, airquality, sound } = req.body;

  if (!sensorId) {
    return res.status(400).send('sensorId est obligatoire');
  }

  // Préparer les valeurs à insérer, par défaut null si absent
  const tempVal = temperature !== undefined ? temperature : null;
  const presVal = pressure !== undefined ? pressure : null;
  const airqVal = airquality !== undefined ? airquality : null;
  const soundVal = sound !== undefined ? sound : null;

  try {
    // Insertion en base
    await pool.query(
      `INSERT INTO sensors (sensor_id, temperature, pressure, airquality, sound) 
       VALUES ($1, $2, $3, $4, $5)`,
      [sensorId, tempVal, presVal, airqVal, soundVal]
    );

    // Envoi à sensors.community
    await sendToSensorsCommunity(req.body, sensorId);

    res.status(201).send('Données enregistrées et envoyées');
  } catch (err) {
    console.error(err);
    res.status(500).send('Erreur serveur');
  }
});

app.listen(port, () => {
  console.log(`API running at http://localhost:${port}`);
});
