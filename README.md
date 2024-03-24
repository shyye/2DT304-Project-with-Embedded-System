# 2DT304-Project-with-Embedded-System
**Course:** 2DT304 Project with Embedded Systems  
**University:** Linnaeus University
## Lifebuoy Monitoring
Student project about lifebuoy monitoring to ensure that lifebuoys are in the correct positions or notice if they are gone. This is due to a wish to be able to replace the need to do manual rounds to check them. The problem formulation is from Kalmar Municipality. The result of this project is a simple representation of a mesh network with Seeed devices that detect whether a life buoy is missing or not and communicate over WiFi with an LoRa device. The LoRa device in its turn sends the data over LoRaWAN to The Things Network (TTN), via MQTT to MongoDB and the data is displayed on a website in real time. 

## Participation
### Both worked with:  
- First prototype test with read button values, ESP-NOW and setup ESP-IDF
- Genereal Manuals / Documentation / Report

### Mattias Stålgren worked with:  
- Mesh implementation (E.g. mostly in ```Seeed.c```)
- Sketch and 3D printing box for the prototype

### Emma Lövgren worked with:
- LoRa / TTN connection (E.g. mostly in ```LoRa.c```)
- Simple website with Node.js, Express, MongoDB, Leaflet.

## Screenshots & Demo(Video)
Youtube Video: [2DT304 Project with Embedded System - Lifebuoy Monitoring Prototype](https://youtu.be/HGqFb-Lzu50?si=iPdqgvS2ZUVpU11g)

## Setup
Links to manuals for this project.

### ESP-IDF
[setup_expressIDF.md](setup_espressIDF.md)

### Hardware
[setup_hardware.md](setup_hardware.md)

### TTN & LoRa
[setup_TTN_LoRa.md](setup_TTN_LoRa.md)

### Decode data from TTN, sent via MQTT to the Website
[setup_decoder_LoRa-TTN-MQTT-Server-Connection.md](setup_decoder_LoRa-TTN-MQTT-Server-Connection.md)

### Website
[setup_website.md](setup_website.md)

