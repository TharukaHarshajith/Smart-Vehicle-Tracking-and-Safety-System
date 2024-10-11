# Smart Vehicle Tracking and Safety System 🚗📍

This repository contains the code for the **Smart Vehicle Tracking and Safety System**, built using **C** and the **ATmega2560 microcontroller**. It features **GPS location tracking**, accident detection via a **vibration sensor**, and **GSM-based SMS alerts** for real-time vehicle safety monitoring.

## Features:
- <u>GPS Location Tracking:</u> Sends the vehicle's real-time location via SMS with a Google Maps link.
- <u>Accident Detection:</u> Monitors a vibration sensor to detect collisions and sends an emergency SMS with the vehicle’s location.
- <u>GSM Communication:</u> Uses the SIM800 module for SMS alerts and location requests.

## Technology Stack:
- **Microcontroller**: ATmega2560
- **Programming Language**: C
- **Modules**: GPS module, GSM module (SIM800), vibration sensor

## How It Works:
- The system monitors the vehicle’s status, checking for collisions and location requests.
- Upon detecting an accident, it sends an SMS to pre-configured contacts with a location link.
- The system can also respond to SMS location requests.

