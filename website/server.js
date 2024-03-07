// TODO: Reference tutorials and write own process in manual
// Source: https://www.youtube.com/watch?v=9OfL9H6AmhQ
// install express
// install nodemon
// install mongoose

// install dotenv https://www.npmjs.com/package/dotenv

// https://www.youtube.com/watch?v=SccSCuHhOw0
// install ejs, vid ca. 7:50 i videon

// For those who don't have node, explain how to install

// npm install mqtt

// Install ejs layouts https://www.npmjs.com/package/express-ejs-layouts
// https://www.youtube.com/watch?v=XlvsJLer_No&list=PLZlA0Gpn_vH8jbFkBjOuFjhxANC63OmXM

// TODO: If we want to avoid the lint errors in the ejs file, we should redo and use this to convert the sent data to a variable in tnside the script tags
// const lifebuoys = JSON.parse('<%- JSON.stringify(lifebuoys) %>')

// npm i methodOvverride: https://www.youtube.com/watch?v=UIf1Lh9OZ-k&list=PLZlA0Gpn_vH8jbFkBjOuFjhxANC63OmXM&index=9


require('dotenv').config()
const express = require('express')
const expressLayouts = require('express-ejs-layouts')
const mainRouter = require('./routes/main')
const lifebuoyRouter = require('./routes/lifebuoys')
const methodOverride = require('method-override')


// Mongoose and Models
const mongoose = require('mongoose')
const Lifebuoy = require('./models/lifebuoyModel') //TODO: 

// Constants
const PORT = 3000
const app = express();


// ChatGPT help and da internet(https://medium.com/kocfinanstech/socket-io-with-node-js-express-5cc75aa67cae) TODO: Look up
const server = require('http').createServer(app);
const io = require('socket.io')(server);
// server.listen(3000, () => {
//     console.log('Server is listening on port 3000');
//   });


// Template engine & Layout & Middleware
app.set('view engine', 'ejs')
app.set('views', __dirname + '/views') // TODO: Why dirname here and not for layouts?
app.set('layout', 'layouts/layout')
app.use(expressLayouts)
app.use(express.static(__dirname + '/public'))  // CSS, JS, images etc.
app.use(express.urlencoded({ extended: false })) //TODO: see if it's necessary to have extended to false
app.use(methodOverride('_method'))  // _method is the variable used where this will be used

app.use(express.json())


// Routes
app.use('/', mainRouter)
app.use('/lifebuoys', lifebuoyRouter)


// TODO: Is this good practice or not?
// Connect to Database
mongoose.connect(process.env.DATABASE_URI)
    .then(() => {
        console.log('Connected to database');

        // app.listen(PORT, () => {
        //     console.log('API running on port 3000');
        // })

        server.listen(PORT, () => {
            console.log('API running on port 3000');
        })

    }).catch((err) => {
        console.log('Error connecting to database', err);
    })

 
// MQTT
const mqtt = require('mqtt')

// Replace these with your TTN MQTT details
// TODO: test TSL How to use secure connection
const ttnMqttBroker = process.env.MQTT_PUBLIC_ADRESS; 
const ttnUsername = process.env.MQTT_USERNAME;
const ttnPassword = process.env.MQTT_PASSWORD;
const ttnApplicationId = process.env.MQTT_APPLICATION_ID;

const client = mqtt.connect(ttnMqttBroker, {
    username: ttnUsername,
    password: ttnPassword,
});

client.on('connect', () => {
    console.log('Connected to TTN MQTT broker');

    // Subscribe to uplink topics for all devices in the application
    const topic = `v3/${ttnUsername}/devices/+/up`;
    client.subscribe(topic, (err) => {
        if (err) {
            console.error('Error subscribing to topic:', err);
        } else {
            console.log(`Subscribed to topic: ${topic}`);
        }
    });
});

client.on('message', (topic, message) => {
    // Handle incoming messages here
    // console.log(`Received message on topic ${topic}: ${message.toString()}`);

    // const payload = message.toString();
    // const jsonData = JSON.parse(payload);
    // console.log('Received JSON data:', jsonData);

    // // Extract payload from the JSON data
    // const base64Payload = jsonData.uplink_message.frm_payload;

    // // Decode base64 payload
    // const decodedBuffer = Buffer.from(base64Payload, 'base64');

    // // Convert buffer to string or handle it according to your data format
    // const decodedString = decodedBuffer.toString('utf-8');

    // console.log(decodedString); // Output: "\x02" (String representation of the hexadecimal value '02')



    // Decode the payload
    const deviceId = 55
    const zone = 55
    const newStatus = 1
    let data = {
        deviceId: deviceId,
        zone: zone
    };

    updateLifebuoy(data, newStatus);
    
    // webhook.emit('data', decodedString); TODO: Check this
    // Emit an event when done
    // lifebuoyEventEmitter.emit('messageReceived', "test");
    // ChatGPT help TODO: Look up
    // Emit an event to all connected Socket.IO clients
    io.emit('mqttMessage', message);
    console.log('Emitting message to all connected clients:', message);
    
    
});

client.on('error', (err) => {
    console.error('MQTT client error:', err);
});

process.on('SIGINT', () => {
    // Close the MQTT connection on SIGINT (Ctrl+C)
    client.end();
    process.exit();
});


// TODO:
function decodePayload(payload) {
    // Decode the payload here
    return payload;
}

async function updateLifebuoy(lifebuoyQuery, status) {

    // Get Lifebuoy from database
    try {
        let lifebuoy = await Lifebuoy.findOne(lifebuoyQuery);
        if (lifebuoy) {
            lifebuoy.status = status;
            await lifebuoy.save();
        } else {
            console.error('No lifebuoy found with the given filters:', lifebuoyQuery);
        }
    } catch (error) {
        console.error('Error occurred while updating Lifebuoy:', error);
    }
}