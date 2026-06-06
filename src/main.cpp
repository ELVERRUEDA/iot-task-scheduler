#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

// CONFIGURA TUS CREDENCIALES
#include "credentials.h"

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;  // Colombia (UTC -5)
const int daylightOffset_sec = 0;

// Pines del motor L298N
const int IN1 = 27;
const int IN2 = 26;
const int ENA = 25;  // Pin habilitado pero controlado por jumper

String ultimaFechaHoraEjecutada = "";
bool tareaEnEjecucion = false;

void encenderMotor() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(ENA, HIGH);
  Serial.println("Motor DC 12V encendido");
}

void detenerMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(ENA, LOW);
  Serial.println("Motor DC 12V detenido");
}

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  detenerMotor();

  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  int intentos = 0;
  while (!getLocalTime(&timeinfo) && intentos < 10) {
    Serial.println("Esperando sincronización de hora...");
    delay(1000);
    intentos++;
  }

  if (intentos == 10) {
    Serial.println(" No se pudo obtener la hora.");
  } else {
    Serial.println(" Hora sincronizada correctamente.");
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado. Reconectando...");
    WiFi.begin(ssid, password);
    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 20) {
      delay(500);
      Serial.print(".");
      intentos++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nReconectado al WiFi");
    } else {
      Serial.println("\nNo se pudo reconectar. Reintentando en 10 segundos.");
      delay(10000);
      return;
    }
    HTTPClient http;
    String url = "https://firestore.googleapis.com/v1/projects/" + String(firebase_project_id) + "/databases/(default)/documents/tareas";
    http.begin(url);

    if (String(authToken).length() > 0) {
      http.addHeader("Authorization", "Bearer " + String(authToken));
    }

    int httpCode = http.GET();

    if (httpCode == 200) {
      String payload = http.getString(); 
      JsonDocument doc; (payload.length() * 2 + 256);
      deserializeJson(doc, payload);

      struct tm timeinfo;
      if (!getLocalTime(&timeinfo)) {
        Serial.println("No se pudo obtener la hora");
        return;
      }

      // Obtener fecha y hora actual
      char currentDateTime[17];
      strftime(currentDateTime, sizeof(currentDateTime), "%Y-%m-%d %H:%M", &timeinfo);
      Serial.print("Fecha y hora actual: ");
      Serial.println(currentDateTime);

      bool tareaEjecutadaEnEsteCiclo = false;

      for (JsonObject tarea : doc["documents"].as<JsonArray>()) {
        String tareaID = tarea["name"];
        tareaID = tareaID.substring(tareaID.lastIndexOf("/") + 1);

        // Obtener fecha y hora de la tarea
        String fecha = tarea["fields"]["fecha"]["stringValue"] | "";
        String hora = tarea["fields"]["hora"]["stringValue"] | "";
        bool completada = tarea["fields"]["completado"]["booleanValue"] | 
                  tarea["fields"]["completada"]["booleanValue"] | false;

        // Combinar fecha y hora para comparación
        String fechaHoraTarea = fecha + " " + hora;

        Serial.println("Tarea ID: " + tareaID);
        Serial.println("Fecha programada: " + fecha);
        Serial.println("Hora programada: " + hora);
        Serial.println("Completada: " + String(completada));

        Serial.println("Comparando: [" + fechaHoraTarea + "] vs [" + String(currentDateTime) + "]");
          if (!completada && fecha != "" && hora != "" && fechaHoraTarea == String(currentDateTime)) {
            if (!tareaEnEjecucion) {
            Serial.println("Ejecutando tarea programada: " + tareaID);
            Serial.println("Fecha y hora de ejecución: " + fechaHoraTarea);
            tareaEnEjecucion = true;
            tareaEjecutadaEnEsteCiclo = true;

            // Encender motor DC por 5 segundos (sin control PWM)
            encenderMotor();
            delay(5000);
            detenerMotor();
            delay(2000);

            // Actualizar tarea como completada en Firestore
            HTTPClient patchHttp;
            String patchPayload = "{\"fields\": {\"completado\": {\"booleanValue\": true}}}";
              String patchUrl = "https://firestore.googleapis.com/v1/projects/" + String(firebase_project_id) + "/databases/(default)/documents/tareas/" + tareaID + "?updateMask.fieldPaths=completado";

            if (String(authToken).length() > 0) {
              patchHttp.addHeader("Authorization", "Bearer " + String(authToken));
            }
            patchHttp.addHeader("Content-Type", "application/json");

            patchHttp.begin(patchUrl);
            int patchCode = patchHttp.PATCH(patchPayload);

            if (patchCode == 200) {
              Serial.println("Tarea marcada como completada");
              ultimaFechaHoraEjecutada = fechaHoraTarea;
            } else {
              Serial.println("Error al actualizar tarea");
              Serial.println(patchHttp.getString());
            }

            patchHttp.end();
          }
        }
      }

      if (!tareaEjecutadaEnEsteCiclo && tareaEnEjecucion) {
        tareaEnEjecucion = false;
        Serial.println("Sin tareas para ejecutar. Motor apagado.");
        detenerMotor();
      } else if (!tareaEjecutadaEnEsteCiclo) {
        Serial.println("Sin tareas programadas para esta fecha y hora.");
      }

    } else {
      Serial.print("Error en la petición GET: ");
      Serial.println(httpCode);
    }

    http.end();
  }

  delay(10000); // Espera 10 segundos antes de verificar nuevamente
}
