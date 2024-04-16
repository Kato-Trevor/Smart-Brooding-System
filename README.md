Team members;
Kato Trevor Thomas   - 2100713085
Kisejjere Rashid     - 2100711543
Aine Daniel Mwejune  -  2100715696
Aparo Cecilia        - 2100705605
Masembe Sendi Joseph - 2100717978
Iraku Harry          - 2100707731

ThingSpeak - https://thingspeak.com/channels/2501148

# Smart-Brooding-System
An IoT-based system that not only monitors the main environmental factors for the chicks’ growth but also keeps the stakeholders informed remotely.

### The Problem
Chicks that have just hatched have underdeveloped bodies that take a lot of energy to adjust to environmental conditions. 
As a result, more of the consumed feeds are converted into energy for body regulation purposes instead of body growth purposes.

### The Solution
Environmental factors such as temperature, humidity, air quality and ventilation have to be continuously regulated to keep the chicks in their comfort zone

### Components
Temperature & Humidity monitoring. The DHT11 sensor will be used to monitor the Temperature and Humidity Levels. These are very important environmental factors and it is encouraged to keep them under the chicks’ comfort zone.

Light Control. The photoresistor will be used to adjust the lighting in the cages according to the light from the environment. Light stimulates the chicks’ growth and development.

Air Quality Monitoring. With the use of the MQ-7 gas sensor, air quality levels will be monitored to ensure ideal conditions especially with the carbon monoxide and ammonia levels which should be less than 10ppm.

Water Level Monitoring. Water levels in the chicks’ water sources will continuously be monitored to ensure adequate water supply. Water is very useful for optimum health of the chicks.

Data Display. The OLED display will display the data collected by the sensors. This will be useful to the farmers as they monitor the environmental conditions.

Data Processing. With the use of the ATMEGA 2560, collected data will be processed and this will be used to ensure that the desired environmental levels are met.

Alert Mechanisms The buzzer will make sound in cases where environmental conditions are extreme that is to say in cases where environmental levels are beyond and below the chicks’ comfort zone.

Connectivity. To allow connectivity to the ThingSpeak platform, the GSM module will be used to send data collected and processed by the sensors and the microcontroller respectively. 
