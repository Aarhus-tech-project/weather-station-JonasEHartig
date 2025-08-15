[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/XBO6NBqk)

Hej

Jeg har lavet opgaven primært i c++ og brugt CMake til at build og compile koden.
På arduino r4 wifi er der Arduino-framework c++ kode på som læser fra en BME280 sensor og sender data til en MQTT_Broker som MQTT_Client.

Serveren er sat op med Ubuntu Linux, og fungere som MQTT_Brokeren og Database. Her på er der en yderligere MQTT_Client som er en server-app lavet med c++ og bygget CMake som sender dataen til en MySQL-Database.

Serveren kører også en Grafana på: 192.168.106.11:3000