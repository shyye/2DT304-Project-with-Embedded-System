# Setup TTN via LoRaWAN

## 1. Create an account at The Things Network (TTN)
Create account at [thethingsnetwork.org](https://www.thethingsnetwork.org/), then follow the steps below.  

## 2. Create application
1. Click on profile name.
2. Choose 'Console' in the dropdown menu.
3. Go to 'Applications' -> Create Application.  
Guide: [Adding Applications](https://www.thethingsindustries.com/docs/integrations/adding-applications/).
4. Select your newly created application -> Click on 'End devices' -> Register end device.  
Guide: [Adding end device](https://www.thethingsindustries.com/docs/devices/adding-devices/).  
Specific steps for this project:  
**Input method**: Select the end device in the LoRaWAN Device Repository  
**Brand**: Heltec (HeltecAutoMation)  
**Model**: WiFi Lora 32(V2) (Class A OTAA)  
**Hardware Ver.**: Unknown  
**Firmware Ver.**: 1.0 (Uncertain about this)  
**Profile (Region)**: EU_863_870  
**Frequency Plan**: Europe 863-870 MHz (SF9 for RX2 - recommended)  
**JoinEUI was set to the general value**: all-zeros.

## 3. Connect end device
In this project the ESP-IDF framework is used.  
To be able to easily send data via LoRaWAN to TTN, add the library ttn-esp32 by Manuel Bl.   
Link: [ttn-esp32](https://github.com/manuelbl/ttn-esp32)  

Follow the **Get Started** guide from the repository above, direct link: [ttn-esp32 - Get Started](https://github.com/manuelbl/ttn-esp32/wiki/Get-Started).  
