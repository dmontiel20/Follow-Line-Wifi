# Smart Robot Car Kit v4.0 — Sigue-Líneas Autónomo con Telemetría IoT (MQTT)

[![Platform: Arduino UNO](https://img.shields.io/badge/Platform-Arduino%20UNO-blue.svg)](https://www.arduino.cc/)
[![Platform: ESP32-CAM](https://img.shields.io/badge/Platform-ESP32--CAM-red.svg)](https://espressif.com/)
[![Protocol: MQTT](https://img.shields.io/badge/IoT-MQTT-green.svg)](https://mqtt.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

Este repositorio contiene la solución completa para el control reactivo y la telemetría en tiempo real del **Smart Robot Car Kit v4.0** (Elegoo), combinando la capacidad de control hardware y control en lazo cerrado (PID) en **Arduino UNO** con las capacidades de conectividad Wi-Fi e IoT sobre **ESP32-CAM**.

---

## 📌 Tabla de Contenidos
- [Descripción del Proyecto](#-descripción-del-proyecto)
- [Arquitectura del Sistema](#-arquitectura-del-sistema)
- [Estructura del Repositorio](#-estructura-del-repositorio)
- [Hardware y Asignación de Pines](#-hardware-y-asignación-de-pines)
- [Comunicación Serie (Arduino ⟷ ESP32)](#-comunicación-serie-arduino--esp32)
- [Protocolo IoT y Mensajería MQTT](#-protocolo-iot-y-mensajería-mqtt)
- [Configuración y Despliegue](#-configuración-y-despliegue)
- [Demostración](#-demostración)
- [Autores](#-autores)

---

## 📖 Descripción del Proyecto

El sistema autónomo cumple con los siguientes requerimientos:
- **Seguimiento de línea de alta velocidad:** Uso del módulo reflectivo ITR20001 (3 canales) en modo analógico continuo para alimentar un algoritmo de control en lazo cerrado (**PID Controller**).
- **Recuperación activa de trayectoria:** Rutina de re-enganche si el vehículo se sale por completo de la línea trazada.
- **Detección y frenada ante obstáculos:** Monitorización por ultrasonidos (HC-SR04) para detener el vehículo a una distancia de seguridad estricta (5 a 8 cm) al completar el circuito.
- **Indicador óptico de estado:** Control direccionable de LED RGB (FastLED) para señalización obligatoria de estado:
  - 🟢 **Verde:** Línea detectada por cualquiera de los 3 sensores.
  - 🔴 **Rojo:** Línea perdida.
- **Puente IoT bidireccional:** El vehículo no inicia el movimiento hasta recibir confirmación por puerto serie de que el ESP32 está conectado a la red Wi-Fi y registrado en el broker MQTT.

---

## 📐 Arquitectura del Sistema

```
 ┌────────────────────────────────────────────────────────┐
 │                   Broker MQTT (TCP)                    │
 │               IP: 193.147.53.2 | Port: 21883           │
 └──────────────────────────▲─────────────────────────────┘
                            │ Wi-Fi (eduroam / Hotspot)
                            ▼
 ┌────────────────────────────────────────────────────────┐
 │                      ESP32-CAM                         │
 │  - Gestión de credenciales Wi-Fi (pass.h)              │
 │  - Cliente Adafruit-MQTT                               │
 │  - Serial Bridge (UART)                                │
 └──────────────────────────▲─────────────────────────────┘
                            │ Interfaz Serial (UART TTL)
                            │ Switch S1 en posición "CAM"
                            ▼
 ┌────────────────────────────────────────────────────────┐
 │                    Arduino UNO                         │
 │  - Lógica de control PID (PID_Controller.h)            │
 │  - Sensores ITR20001 (A0, A1, A2 en analógico)         │
 │  - Sensor Ultrasonidos HC-SR04 (Trig: 13, Echo: 12)    │
 │  - Driver L298N (Motores Grupo A y B + PWM)            │
 │  - LED RGB de depuración de estado (FastLED)           │
 └────────────────────────────────────────────────────────┘
```

---

## 📂 Estructura del Repositorio

```text
.
├── Arduino/
│   └── Arduino.ino          # Firmware de pruebas y control base de periféricos
├── ESP/
│   └── ESP.ino              # Firmware base de enlace WiFi y puente serie para ESP32
├── p4_setr/
│   ├── p4_setr.ino          # Firmware principal: bucle de control, FSM y actuadores
│   └── PID_Controller.h     # Implementación del algoritmo PID para lectura analógica de línea
├── wifiArduino/
│   └── wifiArduino.ino      # Pruebas de sincronización y handshake con ESP vía serie
├── wifiESP/
│   ├── pass.h               # Configuración de credenciales Wi-Fi y parámetros del broker
│   └── wifiESP.ino          # Conexión Wi-Fi y cliente MQTT en ESP32
├── WhatsApp Video ...mp4    # Demostración del robot en pista
└── README.md                # Documentación del proyecto
```

---

## 🔌 Hardware y Asignación de Pines

### 1. Sensores Infrarrojos de Línea (ITR20001)
> **Nota:** Se procesan en **rango analógico continuo (0 a 1023)** para calcular el error continuo del algoritmo PID, maximizando la fluidez del giro.

| Sensor | Pin Arduino | Modo |
| :--- | :--- | :--- |
| **ITR20001 Izquierdo** | `A2` | Analógico |
| **ITR20001 Central** | `A1` | Analógico |
| **ITR20001 Derecho** | `A0` | Analógico |

### 2. Sensor de Distancia por Ultrasonidos
| Señal | Pin Arduino | Función |
| :--- | :--- | :--- |
| **TRIG_PIN** | `13` | Disparo del pulso ultrasónico |
| **ECHO_PIN** | `12` | Entrada del pulso de eco |

### 3. Driver de Motores DC
| Señal | Pin Arduino | Tipo | Descripción |
| :--- | :--- | :--- | :--- |
| `PIN_Motor_STBY` | `3` | Digital | Standby (HIGH: Activo, LOW: Desactivado) |
| `PIN_Motor_AIN_1` | `7` | Digital | Dirección Motores Derecho (HIGH: Adelante, LOW: Atrás) |
| `PIN_Motor_PWMA` | `5` | PWM (0-255) | Control de velocidad Motores Derecho |
| `PIN_Motor_BIN_1` | `8` | Digital | Dirección Motores Izquierdo (HIGH: Adelante, LOW: Atrás) |
| `PIN_Motor_PWMB` | `6` | PWM (0-255) | Control de velocidad Motores Izquierdo |

---

## 🔄 Comunicación Serie (Arduino ⟷ ESP32)

* **Interbloqueo de arranque:** El robot no avanza hasta que el ESP32 envía una confirmación por UART indicando que la conexión con el broker MQTT se ha establecido con éxito.
* **Manejo del Switch S1 en la Shield:**
  * **Modo `UPLOAD`:** Para programar el microcontrolador Arduino UNO por USB evitando conflictos en las líneas RX/TX.
  * **Modo `CAM`:** Para activar la comunicación serie física UART entre el ATmega328P y el módulo ESP32-CAM durante la ejecución normal.

---

## 🌐 Protocolo IoT y Mensajería MQTT

### Parámetros de Red
* **Broker Host:** `193.147.53.2`
* **Puerto:** `21883`
* **Topic base:** `/SETR/2023/<ID_EQUIPO>/`

### Esquema de Mensajes (JSON Payloads)

#### 1. Inicio de Vuelta (`START_LAP`)
Emitido exactamente una vez justo antes de iniciar la tracción de los motores.
```json
{
  "team_name": "$TU_NOMBRE_DE_EQUIPO",
  "id": "$ID_EQUIPO",
  "action": "START_LAP"
}
```

#### 2. Fin de Vuelta (`END_LAP`)
Emitido al concluir el circuito tras detectar el obstáculo final. El parámetro `time` reporta los milisegundos transcurridos desde `START_LAP`.
```json
{
  "team_name": "$TU_NOMBRE_DE_EQUIPO",
  "id": "$ID_EQUIPO",
  "action": "END_LAP",
  "time": 45320
}
```

#### 3. Obstáculo Detectado (`OBSTACLE_DETECTED`)
Emitido cada vez que se detecta un obstáculo en el rango seguro de 5 a 8 cm.
```json
{
  "team_name": "$TU_NOMBRE_DE_EQUIPO",
  "id": "$ID_EQUIPO",
  "action": "OBSTACLE_DETECTED",
  "distance": 6.4
}
```

#### 4. Línea Perdida (`LINE_LOST`)
Se genera cuando ninguno de los tres sensores infrarrojos detecta la línea negra.
```json
{
  "team_name": "$TU_NOMBRE_DE_EQUIPO",
  "id": "$ID_EQUIPO",
  "action": "LINE_LOST"
}
```

#### 5. Latido Periódico (`PING`)
Mensaje emitido cada 4 segundos indicando el tiempo de operación sin bloquear el lazo de control reactivo.
```json
{
  "team_name": "$TU_NOMBRE_DE_EQUIPO",
  "id": "$ID_EQUIPO",
  "action": "PING",
  "time": 8000
}
```

#### 6. Mensajes Opcionales Implementados
* **`INIT_LINE_SEARCH` / `STOP_LINE_SEARCH`:** Marcan el inicio y fin de la subrutina de búsqueda activa tras salirse del carril.
* **`LINE_FOUND`:** Se emite únicamente cuando la línea vuelve a ser visible tras un estado `LINE_LOST`.
* **`VISIBLE_LINE`:** Métrica global transmitida al finalizar la vuelta con el porcentaje de lecturas positivas de línea respecto al total muestreado.
  ```json
  {
    "team_name": "$TU_NOMBRE_DE_EQUIPO",
    "id": "$ID_EQUIPO",
    "action": "VISIBLE_LINE",
    "value": 92.45
  }
  ```

---

## 🚀 Configuración y Despliegue

### Requisitos y Librerías
* **Arduino IDE** configurado con el gestor de tarjetas para ESP32:
  ```text
  [https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json](https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json)
  ```
* **Placa ESP32:** Seleccionar `AI Thinker ESP32-CAM`.
* **Librerías Arduino necesarias:**
  - `FastLED`
  - `Adafruit MQTT Library`
  - `ArduinoJson`

### Pasos de Carga
1. **Arduino UNO:**
   - Sitúa el conmutador **S1 en posición `UPLOAD`**.
   - Conecta el cable USB, abre el proyecto `p4_setr/p4_setr.ino` y realiza la carga.
2. **ESP32-CAM:**
   - Ajusta los parámetros de red en `wifiESP/pass.h`.
   - Conecta el módulo ESP32 mediante adaptador FTDI en el puerto correspondiente (ej. `/dev/ttyUSB0`).
   - Abre `wifiESP/wifiESP.ino` y sube el firmware.
3. **Puesta en marcha:**
   - Desconecta los programadores y conmuta el interruptor **S1 a la posición `CAM`**.
   - Enciende el robot con la batería para iniciar el intercambio de sincronización serie y la navegación autónoma.

### Monitorización remota MQTT
Para supervisar en vivo los eventos emitidos por el vehículo:
```bash
mosquitto_sub -v -h 193.147.53.2 -p 21883 -t /SETR/2023/<ID_EQUIPO>/
```

---

## 📹 Demostración

El repositorio contiene el archivo `WhatsApp Video 2024-12-19 at 12.01.36.mp4` que documenta el comportamiento dinámico en pista: seguimiento en curvas, control de tracción proporcional, cambio del LED RGB en tiempo real y parada limpia ante el obstáculo.

---

## 👥 Autores

Proyecto realizado en el marco de la asignatura **Sistemas Empotrados y de Tiempo Real (SETR)**.
- Integrante 1 — [@usuario1](https://github.com/usuario1)
- Integrante 2 — [@usuario2](https://github.com/usuario2)
