const express = require('express');
const app = express();
const port = 3000;

app.use(express.json());

app.get('/', (req, res) => {
    res.send('Bienvenue sur l\'API Node.js !');
});

app.get('/data', (req, res) => {
    const data = {
        temperature: 25.5,
        pressure: 1013.25,
        airQuality: 42,
        soundLevel: 65.3
    };
    res.json(data);
});

app.post('/data', (req, res) => {
    console.log('Données reçues :', req.body);
    res.status(201).send('Données enregistrées avec succès.');
});

app.listen(port, () => {
    console.log(`API en cours d'exécution sur http://localhost:${port}`);
});
