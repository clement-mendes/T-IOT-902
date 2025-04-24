const express = require('express');
const app = express();
const port = 3000;

app.use(express.json());

// Route to handle the root endpoint
app.get('/', (req, res) => {
    res.send('Welcome to the Node.js API!');
});

// Route to retrieve data
app.get('/data', (req, res) => {
    const data = {
        temperature: 25.5,
        pressure: 1013.25,
        airQuality: 42,
        soundLevel: 65.3
    };
    res.json(data);
});

// Route to handle data submission
app.post('/data', (req, res) => {
    console.log('Data received:', req.body);
    res.status(201).send('Data successfully saved.');
});

app.listen(port, () => {
    console.log(`API running at http://localhost:${port}`);
});
