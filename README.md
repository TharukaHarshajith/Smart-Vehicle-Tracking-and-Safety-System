Smart Vehicle Tracking and Safety System 🚗📍
This repository contains the source code for the Smart Vehicle Tracking and Safety System, an embedded system designed to enhance vehicle safety and improve accident detection. The system provides real-time vehicle location tracking and automated alerts in the event of an emergency or collision.

Features:
GPS Location Tracking: Sends the vehicle's real-time location via SMS with a Google Maps link.
Accident Detection: Monitors a vibration sensor to detect collisions and sends an emergency SMS with the vehicle’s location.
GSM Communication: Uses the SIM800 module to handle SMS-based communication for alerts and location requests.
Buzzer Activation: Triggers a buzzer during emergency situations to indicate the system is activated.
Technology Stack:
Microcontroller: ATmega2560
Programming Language: C
Modules: GPS module, GSM module (SIM800), vibration sensor
How It Works:
The system continuously monitors the vehicle’s status, checking for collision events and location requests.
Upon detecting an accident, it sends an SMS to pre-configured contacts, containing the location link.
The system can also respond to SMS requests to share the vehicle’s current location.
