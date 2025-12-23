# Freezer Monitoring System - ESP32 Firmware

🧊 Smart IoT Freezer with AI-Based Monitoring & Control

A full-stack IoT + AI smart freezer system that integrates real-time hardware sensing, load-aware adaptive control, cloud dashboards, and machine-learning–based intelligence for inventory detection and predictive maintenance.

This project demonstrates an end-to-end closed-loop architecture, from physical freezer hardware to AI-driven control decisions.

📌 Project Overview

Traditional freezer systems operate using fixed or reactive control logic, leading to inefficient energy usage, delayed fault detection, and poor inventory visibility.
This project addresses those limitations by introducing:

Edge-level real-time control using ESP32

Cloud-based monitoring and data logging

Vision-based inventory detection using deep learning

Machine learning–based predictive maintenance

Load-aware compressor and EEV control

The system is designed to be scalable, explainable, and industry-aligned.

🏗️ System Architecture

The system follows a three-layer architecture:

1️⃣ Edge IoT Layer (Inside the Freezer)

Responsible for sensing, control, and safety-critical operations.

2️⃣ Cloud IoT Layer

Handles data storage, visualization, alerts, and communication.

3️⃣ AI / Analytics Layer

Provides intelligence such as inventory detection, anomaly detection, and optimization insights.

All layers are connected in a closed-loop feedback system.

🔧 Hardware Components
Controller

ESP32 / ESP32-S3

Sensors

DS18B20 – Cabinet & evaporator temperature

DHT22 – Humidity

Reed switch – Door status

SW-420 – Vibration sensing

INA219 – Current & power monitoring

High-side & low-side pressure sensors

Suction & discharge line temperature sensors

ESP32-CAM – Image capture for inventory detection

Actuators

Compressor speed control module (RPM control)

Electronic Expansion Valve (EEV) with stepper driver

Condenser fan PWM driver

Relay / SSR for safety shutdown

⚙️ Edge-Level Control Logic

The ESP32 executes real-time control independent of cloud connectivity.

Key Functions

Sensor fusion and filtering

PID-based cabinet temperature control

Load-aware compressor speed control

Superheat-based EEV modulation

Door-event compensation logic

Safety shutdown (overcurrent, vibration, pressure faults)

TinyML-based anomaly detection

Data packaging and cloud upload

📊 Cloud IoT Layer
Technologies Used

MQTT (Mosquitto / HiveMQ / AWS IoT)

Firebase / Firestore

Cloud Storage (for images)

Node-RED / Grafana dashboards

Cloud Functions

Real-time data visualization

Historical data logging

Alerts (faults, anomalies, thresholds)

Multi-freezer monitoring support

OTA firmware update support

The cloud layer does not participate in real-time control, ensuring system safety and low latency.

🤖 AI & Machine Learning Layer
🔍 Vision-Based Inventory Detection

Images captured using ESP32-CAM

Uploaded to cloud storage

Processed using a YOLO-based object detection model

Inventory count estimated and logged

Inventory data used as an input for load estimation

🛠️ Predictive Maintenance

ML models analyze:

Vibration trends

Power consumption patterns

Temperature behavior over time

Detects abnormal operating conditions

Generates health indicators and warnings

🧠 Role of ML

ML acts as a supervisory intelligence layer, not direct control:

Detects anomalies

Restricts unsafe control actions

Improves reliability and safety

🔄 Load-Aware Control Strategy

Since refrigeration load cannot be measured directly, the system infers load using:

Cabinet temperature dynamics

Power consumption

Door opening events

OCR/vision-based inventory count

A Load Index is computed and used to adapt:

Compressor RPM

EEV position

Condenser fan speed

This enables predictive, energy-efficient control rather than reactive ON/OFF operation.
