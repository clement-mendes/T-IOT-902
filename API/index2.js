// const express = require('express');
// const app = express();
// const port = 3000;

// const pool = require('./db');

// app.use(express.json());

// app.get('/', (req, res) => {
//     res.send('Welcome to the Node.js API!');
// });

// // Lire des données depuis PostgreSQL
// app.get('/data', async (req, res) => {
//     try {
//         const result = await pool.query('SELECT * FROM sensors ORDER BY timestamp DESC LIMIT 1');
//         res.json(result.rows[0]);
//     } catch (err) {
//         console.error('Erreur lors de la récupération des données:', err);
//         res.status(500).send('Erreur serveur');
//     }
// });

// app.post('/data', async (req, res) => {
//     console.log('Reçu :', req.body); // Allows you to see the incoming request body
  
//     const { temperature, pressure, airQuality, soundLevel } = req.body; // Be sure to destructure the body
  
//     try {
//       await pool.query( 
//         'INSERT INTO sensors (temperature, pressure, air_quality, sound_level) VALUES ($1, $2, $3, $4)',
//         [temperature, pressure, airQuality, soundLevel]
//       );
//       res.status(201).send('Data successfully saved.');
//     } catch (err) {
//       console.error('Erreur lors de l\'insertion des données:', err);
//       res.status(500).send('Erreur serveur');
//     }
// });


// app.listen(port, () => {
//     console.log(`API running at http://localhost:${port}`);
// });
