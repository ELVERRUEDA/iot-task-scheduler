# 🪟 Sistema de Cortina Automática + Programador de Tareas IoT

Sistema completo de automatización que combina hardware ESP32, una API REST, un frontend React y flujos de automatización con n8n para controlar una cortina motorizada y recibir notificaciones en Telegram.

---

## 📐 Arquitectura del sistema

```
┌─────────────────┐     ┌──────────────────┐     ┌─────────────────┐
│  Frontend React  │────▶│  FastAPI + Docker │────▶│    Firebase     │
│  (Programador)   │     │  Puerto 8080      │     │   Firestore     │
└─────────────────┘     └──────────────────┘     └────────┬────────┘
                                                           │
                         ┌─────────────────┐              │
                         │   ESP32          │◀─────────────┘
                         │   + L298N        │  Lee tareas programadas
                         │   + Motor 12V    │  vía Firestore REST API
                         └─────────────────┘
                                  │
                         Ejecuta motor al cumplirse
                         fecha y hora programada
                                  │
                         ┌─────────────────┐     ┌─────────────────┐
                         │      n8n         │────▶│    Telegram     │
                         │  (Docker local)  │     │  Notificaciones │
                         └─────────────────┘     └─────────────────┘
```

---

## ✨ Funcionalidades

- 📅 Programa tareas con título, descripción, fecha y hora desde el frontend React
- ⚙️ El ESP32 consulta Firebase cada 10 segundos y ejecuta el motor al cumplirse la tarea
- ✅ Marca automáticamente la tarea como completada en Firebase tras la ejecución
- 🔔 n8n monitorea las tareas cada minuto y envía notificaciones a Telegram:
  - 🔴 Tarea pendiente detectada
  - ✅ Tarea completada por el ESP32
- 🐳 FastAPI y n8n corren en Docker para despliegue reproducible

---

## 🛠️ Stack tecnológico

| Componente | Tecnología |
|---|---|
| Backend API | Python + FastAPI |
| Base de datos | Firebase Firestore |
| Frontend | React + JavaScript |
| Hardware | ESP32 + L298N + Motor DC 12V |
| Automatización | n8n |
| Contenedores | Docker |
| Notificaciones | Telegram Bot API |

---

## 📁 Estructura del proyecto

```
cortina-automatica-esp32/
├── app/
│   ├── main.py                  # Entrada FastAPI
│   ├── routes.py                # Endpoints CRUD de tareas
│   ├── models.py                # Modelos Pydantic
│   ├── database.py              # Conexión Firebase
│   └── firebase_credentials.json  # ⚠️ No incluido (ver .gitignore)
├── src/
│   └── main.cpp                 # Firmware ESP32
├── requirements.txt
├── .gitignore
└── README.md
```

---

## 🚀 Instalación y uso

### 1. Clonar el repositorio
```bash
git clone https://github.com/ELVERRUEDA/cortina-automatica-esp32.git
cd cortina-automatica-esp32
```

### 2. Configurar Firebase
- Crea un proyecto en [Firebase Console](https://console.firebase.google.com)
- Descarga las credenciales del servicio y guárdalas como `app/firebase_credentials.json`

### 3. Levantar la API con Docker
```bash
# Levantar FastAPI
docker run -d -p 8080:8080 -v $(pwd):/app tu-imagen-fastapi

# Levantar n8n
docker run -d --name n8n -p 5678:5678 \
  -v ~/.n8n:/home/node/.n8n \
  n8nio/n8n
```

### 4. Levantar el frontend
```bash
cd frontend_fast_api
npm install
npm start
```

### 5. Configurar el ESP32
- Abre `src/main.cpp` en PlatformIO
- Configura tu SSID, password y `firebase_project_id`
- Flashea el firmware al ESP32

---

## 🔌 Endpoints de la API

| Método | Endpoint | Descripción |
|---|---|---|
| GET | `/tareas/` | Obtener todas las tareas |
| POST | `/tareas/` | Crear nueva tarea |
| PUT | `/tareas/{id}` | Editar tarea |
| DELETE | `/tareas/{id}` | Eliminar tarea |
| PATCH | `/tareas/{id}/completado` | Marcar como completada |

---

## ⚙️ Conexión del hardware

| ESP32 Pin | L298N | Descripción |
|---|---|---|
| GPIO 27 | IN1 | Dirección motor |
| GPIO 26 | IN2 | Dirección motor |
| GPIO 25 | ENA | Habilitación |
| GND | GND | Tierra común |

---

## 📋 Variables de entorno requeridas

```
FIREBASE_PROJECT_ID=tu-proyecto-firebase
```

> ⚠️ Nunca subas `firebase_credentials.json` a GitHub. Está incluido en `.gitignore`.

---

## 👤 Autor

**Elver Rueda** — [@ELVERRUEDA](https://github.com/ELVERRUEDA)
