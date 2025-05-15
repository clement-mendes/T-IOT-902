const express = require('express');
const fetch = require('node-fetch');
const pool = require('./db'); // ta connexion PostgreSQL

const app = express();
const port = 3000;

app.use(express.json());

const sensorGMP280 = 56391; // Remplace par ton vrai sensor ID sensors.community

async function sendToSensorsCommunity(data, sensorId) {
  const body = {
    software_version: "nodejs_api_1.0",
    sensordatavalues: [
      { value_type: "temperature", value: data.temperature.toString() },
      { value_type: "pressure", value: data.pressure.toString() },
      { value_type: "air_quality", value: data.airQuality.toString() },
      { value_type: "sound_level", value: data.soundLevel.toString() },
      // Ajoute d'autres si tu en as (ex: humidity, PM1, PM2.5, etc.)
    ]
  };

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
  const { temperature, pressure, airQuality, soundLevel } = req.body;

  if (
    temperature === undefined ||
    pressure === undefined ||
    airQuality === undefined ||
    soundLevel === undefined
  ) {
    return res.status(400).send('Données manquantes');
  }

  try {
    // 1. Insérer en base PostgreSQL
    await pool.query(
      'INSERT INTO sensors (temperature, pressure, air_quality, sound_level) VALUES ($1, $2, $3, $4)',
      [temperature, pressure, airQuality, soundLevel]
    );

    // 2. Envoyer à sensors.community
    const sensorId = 12345; // remplace par ton vrai sensor ID sensors.community
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
