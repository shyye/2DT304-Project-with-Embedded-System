# Setup decoder

## 1. Incoming Data
The Seeed or LoRa devices sends sensor data.  
The LoRa device sends the data to The Things Network (TTN)

## 2. TTN sends data via MQTT
Server.js includes code that connects to TTN via MQTT (not the secure one yet).  
When a message arrives, the server decodes the message.

## 3.  Decoding
The recieved message looks something like the below JSON data.  
The **frm_payload** is the message sent, in base64 format.
```
Received JSON data: {
  end_device_ids: {
    device_id: 'YourDeviceId',
    application_ids: { application_id: 'YourApplicationId' },
    dev_eui: 'YourDevEUI',
    join_eui: 'YourJoinEUI'
  },
  correlation_ids: [
    ...
    ...
  ],
  received_at: '2024-03-07T21:22:52.872345578Z',
  uplink_message: {
    f_port: 1,
    frm_payload: 'AQID',
    rx_metadata: [ [Object] ],
    settings: { data_rate: [Object], frequency: '868000000' }
  },
  simulated: true
}
```
The decoding steps done in this project are:  
```
const data = JSON.parse(message.toString());            // Convert to JSON object to be able to extract the payload
const payload = data.uplink_message.frm_payload;        // Get the raw payload that is in base64
const bytes = Buffer.from(payload, 'base64');           // Convert it to bytes
```

