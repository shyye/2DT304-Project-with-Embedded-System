# Setup Website
The website implementation includes a connection to MongoDB online version and connection to MQTT (via The Things Network).  
To be able to use it you need to create a ".env" file in the website folder.  
The file should include the following information:  

```
#  MongoDB URI
DATABASE_URI = "YourDatabaseURI"

# TTN Application
MQTT_PUBLIC_ADRESS = "AdressFromTTN"
MQTT_USERNAME = "TTNApplicationUsername"
MQTT_PASSWORD = "???????????????"
MQTT_APPLICATION_ID = "???????????"
```
