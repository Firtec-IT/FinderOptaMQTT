/* -------------------------------------------------------- */
/*                        FIRTEC.IT                         */
/*            COMUNICAZIONE MQTT CON FINDER OPTA            */
/*                      V. 1.0 290525                       */
/* -------------------------------------------------------- */

#include "opta_info.h"
#include <Ethernet.h>
#include <PubSubClient.h>

bool DEBUG = true;

/* --------------------------------------------------------- */
/*                        SERVER MQTT                        */
/* --------------------------------------------------------- */

const char* mqtt_server = "192.168.xxx.xxx";
const int mqtt_port = 1883;
const char* mqtt_client_id = "FinderOptaClient";
const char* mqtt_user = "xxxxxxxxxx";
const char* mqtt_password = "xxxxxxxxxx";

OptaBoardInfo* info;
OptaBoardInfo* boardInfo();

EthernetClient ethernetClient;
PubSubClient client(ethernetClient);

/* -------------------------------------------------------- */
/*                      INPUT / OUTPUT                      */
/* -------------------------------------------------------- */

// Input digitali
const int OPTA_DIGITAL_IN[] = {A0, A1, A2, A3, A4, A5, A6};
const char* digInputPublishTopics[] = {
  "opta/input/1/stato",
  "opta/input/2/stato",
  "opta/input/3/stato",
  "opta/input/4/stato",
  "opta/input/5/stato",
  "opta/input/6/stato",
  "opta/input/7/stato"
};
const char* digInputSubscribeTopics[] = {
  "opta/input/1",
  "opta/input/2",
  "opta/input/3",
  "opta/input/4",
  "opta/input/5",
  "opta/input/6",
  "opta/input/7"
};
int numDigInput = sizeof(OPTA_DIGITAL_IN) / sizeof(OPTA_DIGITAL_IN[0]);

// Input analogici
const int OPTA_ANALOG_IN[] = {A7};
const char* analogInputPublishTopics[] = {
  "opta/input/8/stato",
};
const char* analogInputSubscribeTopics[] = {
  "opta/input/8",
};
int numAnalogInput = sizeof(OPTA_ANALOG_IN) / sizeof(OPTA_ANALOG_IN[0]);
unsigned long ts_ultimo_msg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
float prevVoltage = 0;

// Output
const int OPTA_OUTPUT[] = {D0, D1, D2, D3};
const char* relayPublishTopics[] = {
  "opta/relay/1/stato",
  "opta/relay/2/stato",
  "opta/relay/3/stato",
  "opta/relay/4/stato"
};
const char* relaySubscribeTopics[] = {
  "opta/relay/1",
  "opta/relay/2",
  "opta/relay/3",
  "opta/relay/4"
};
int numRelays = sizeof(OPTA_OUTPUT) / sizeof(OPTA_OUTPUT[0]);

/* --------------------------------------------------------- */
/*                  PAYLOAD DAL SERVER MQTT                  */
/* --------------------------------------------------------- */

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  if (DEBUG) {
    Serial.print(">>>>>>> Messaggio ricevuto - ");
    Serial.print(topic);
    Serial.print(": ");
  }
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  for (int i = 0; i < numRelays; i++) {
    if (strcmp(topic, relaySubscribeTopics[i]) == 0) {
      if (message == "ON") {
        if (DEBUG) {
          Serial.print("RELAY D");
          Serial.print(i);
          Serial.println(" ON");
        }
        digitalWrite(OPTA_OUTPUT[i], HIGH); // Output ON
        client.publish(relayPublishTopics[i], "ON"); // Pubblica lo stato aggiornato
      } else if (message == "OFF") {
        if (DEBUG) {
          Serial.print("RELAY D");
          Serial.print(i);
          Serial.println(" OFF");
        }
        digitalWrite(OPTA_OUTPUT[i], LOW); // Output OFF
        client.publish(relayPublishTopics[i], "OFF"); // Pubblica lo stato aggiornato
      }
    }
  }
}

/* --------------------------------------------------------- */
/*                   CONNESSIONE AL SERVER                   */
/* --------------------------------------------------------- */

void reconnect() {
  while (!client.connected()) {
    if (DEBUG) { Serial.print("Tentativo di connessione al server MQTT..."); }
    if (client.connect(mqtt_client_id, mqtt_user, mqtt_password)) {
      if (DEBUG) { Serial.println("Connesso"); }
      // Accende il LED_D1 per indicare che il dispositivo è connesso al server MQTT
      digitalWrite(LED_D1, HIGH);
      // Sottoscrizione ai topic
      for (int i = 0; i < numRelays; i++) {
        client.subscribe(relaySubscribeTopics[i]);
        if (DEBUG) {
          Serial.print("Sottoscritto al topic: ");
          Serial.println(relaySubscribeTopics[i]);
        }
      }
    } else {
      // Spegne il LED_D1 per indicare che il dispositivo NON è connesso al server MQTT
      digitalWrite(LED_D1, LOW);
      if (DEBUG) {
        Serial.print("Errore, rc=");
        Serial.println(client.state());
      }
      // Attendi 5 secondi prima di riprovare
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  
  analogReadResolution(12);
  // LED
  pinMode(LED_D0, OUTPUT);
  pinMode(LED_D1, OUTPUT);
  pinMode(LED_D2, OUTPUT);
  pinMode(LED_D3, OUTPUT);
  // Relay Output
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);

  // Connessione alla rete
  if (boardInfo()->magic == 0xB5) {
    if (DEBUG) { Serial.print("Tentativo di connessione alla rete ethernet..."); }
    while (Ethernet.begin(boardInfo()->mac_address) == 0) {
      // Connessione fallita
      digitalWrite(LED_D0, HIGH);
      delay(500);
      digitalWrite(LED_D0, LOW);
    }
    if (DEBUG) { Serial.println("Connesso"); }
    // Accende il LED_D0 per indicare che il dispositivo è connesso alla rete ethernet
    digitalWrite(LED_D0, HIGH);

    // Configurazione del server MQTT e della callback
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(mqttCallback);
  } else {
    if (DEBUG) { Serial.print("Impossibile connettersi alla rete ethernet"); }
  }
}

void loop() {
  // Controllo della connessione al server MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Pubblica un messaggio ogni secondo
  unsigned long timestamp = millis();
  if (timestamp - ts_ultimo_msg > 1000) {
    ts_ultimo_msg = timestamp;

    // Digital Input
    if (DEBUG) { Serial.print("INPUT DIGITALI: "); }
    for (int i = 0; i < numDigInput; i++) {
      int inputState = digitalRead(OPTA_DIGITAL_IN[i]);
      if (inputState == HIGH) {
        client.publish(digInputPublishTopics[i], "ON");
        if (DEBUG) { Serial.print("1 "); }
      } else {
        client.publish(digInputPublishTopics[i], "OFF");
        if (DEBUG) { Serial.print("0 "); }
      }
    }
    if (DEBUG) { Serial.println(""); }

    // Analog Input
    if (DEBUG) { Serial.print("INPUT ANALOGICI: "); }
    const float ANALOG_VOLTAGE_CHANGE_THRESHOLD = 0.10;
    for (int i = 0; i < numAnalogInput; i++) {
      // Variabili per il calcolo della tensione
      float VOLTAGE_MAX = 3.3;
      float RESOLUTION = 4095.0;
      float DIVIDER = 0.3034;
      // Valore input analogico
      int terminalValue = analogRead(OPTA_ANALOG_IN[i]);
      float currentVoltage = terminalValue * (VOLTAGE_MAX / RESOLUTION) / DIVIDER;
      if (prevVoltage != currentVoltage && abs(currentVoltage - prevVoltage) >= ANALOG_VOLTAGE_CHANGE_THRESHOLD) {
        // Pubblica il valore solo se supera il valore minimo
        prevVoltage = currentVoltage;
        snprintf(msg, MSG_BUFFER_SIZE, "%.2f", currentVoltage);
        client.publish(analogInputPublishTopics[i], msg); 
      }
      if (DEBUG) {
        Serial.print(prevVoltage);
        Serial.print("v ");
      }
    }
    if (DEBUG) { Serial.println(""); }

    // Output
    if (DEBUG) { Serial.print("OUTPUT: "); }
    for (int i = 0; i < numRelays; i++) {
      int relayState = digitalRead(OPTA_OUTPUT[i]);
      if (relayState == HIGH) {
        client.publish(relayPublishTopics[i], "ON");
        if (DEBUG) { Serial.print("1 "); }
      } else {
        client.publish(relayPublishTopics[i], "OFF");
        if (DEBUG) { Serial.print("0 "); }
      }
    }
    if (DEBUG) {
      Serial.println("");
      Serial.println("==========================");
    }

    // Il led LED_D2 Lampeggia per indicare la comunicazione con il server MQTT
    digitalWrite(LED_D2, !digitalRead(LED_D2));
  }

  delay(200);
}