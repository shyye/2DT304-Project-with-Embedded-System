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

client.on('message', async (topic, message)  => {
    
    const data = decodePayload(message)     // Store payload in array
    const object = convertData(data)

    // console.log(data)
    // console.log(object)

    const lifebuoy = await updateLifebuoy(object.lifebuoyQuery, object.newStatus)

    // console.log(lifebuoy);
    // console.log(lifebuoy.id);

    // Decode the payload TODO:
    // const deviceId = 55
    // const zone = 55
    // const newStatus = 1
    // let data = {
    //     deviceId: deviceId,
    //     zone: zone
    // }; 

    console.log("in server.js", lifebuoy);

    // ChatGPT help TODO: Look up
    // Emit an event to all connected Socket.IO clients
    io.emit('mqttMessage', lifebuoy);
        
})

client.on('error', (err) => {
    console.error('MQTT client error:', err);
});

process.on('SIGINT', () => {
    // Close the MQTT connection on SIGINT (Ctrl+C)
    client.end();
    process.exit();
});



/**
 * Decode payload from base64 to bytes
 * @param {*} message message from MQTT that can be converted to JSON
 * @returns An array containing all bytes from the payload
 */
function decodePayload(message) {
    const payloadArray = []

    // Handle incoming messages here
    // console.log(`Received message on topic ${topic}: ${message.toString()}`);
    // const payload = message.toString();
    // const jsonData = JSON.parse(payload);
    // console.log('Received JSON data:', jsonData);

    const data = JSON.parse(message.toString());            // Convert to JSON object to be able to extract the payload
    const payload = data.uplink_message.frm_payload;        // Get the raw payload that is in base64
    const bytes = Buffer.from(payload, 'base64');           // Convert it to bytes

    for (let i = 0; i < bytes.length; i++) {
        let value = bytes.readInt8(i);
        payloadArray.push(value);
    }   
    return payloadArray;
}

/**
 * Takes the data sent from devices (the payload from TTN) 
 * and converts to data that can be used to identify the 
 * corresponding lifebuoy in the database.
 * @param {*} payloadArray An array containing all bytes from the payload
 * @returns
 *     lifebuoyQuery - Object with deviceId and zone to identify the lifebuoy
 *     newStatus - The value of the lifebuoy status (1 or 0 / In Place or Missing)
 */
function convertData(payloadArray) {
    const deviceId = payloadArray[0];
    const zone = payloadArray[1];
    const newStatus = payloadArray[2];

    let lifebuoyQuery = {
        deviceId: deviceId,
        zone: zone
    }
    return {lifebuoyQuery, newStatus}
}

async function updateLifebuoy(lifebuoyQuery, status) { 
    try {
        // Get Lifebuoy from database
        let lifebuoy = await Lifebuoy.findOne(lifebuoyQuery);
        if (lifebuoy) {
            lifebuoy.status = status
            await lifebuoy.save()

            return lifebuoy
        } else {
            console.error('No lifebuoy found with the given filters:', lifebuoyQuery)
        }
    } catch (error) {
        console.error('Error occurred while updating Lifebuoy:', error)
    }
}