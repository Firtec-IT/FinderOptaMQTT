# Comunicazione MQTT con Finder Opta

Questo repository ospita uno sketch Arduino progettato per abilitare la comunicazione **MQTT (Message Queuing Telemetry Transport)** con un dispositivo **Finder Opta**. Permette al Finder Opta di pubblicare lo stato dei suoi input digitali e analogici su un broker MQTT e di sottoscriversi a topic specifici per controllare i suoi output (relay).

## Funzionalità Principali

Lo sketch offre una serie di funzionalità chiave per l'integrazione del Finder Opta in un sistema MQTT:

* **Monitoraggio Input Digitali**:
    * Legge lo stato degli input digitali configurati (es. `A0` - `A6`).
    * Pubblica lo stato (`"ON"` o `"OFF"`) su topic MQTT dedicati (es. `opta/input/1/stato`).

* **Lettura Input Analogici**:
    * Acquisisce i valori dall'input analogico configurato (es. `A7`).
    * Converte i valori raw in **tensione (Volt)**.
    * Pubblica i dati su un topic MQTT (es. `opta/input/8/stato`). La pubblicazione avviene solo se la variazione di tensione supera una soglia predefinita.

* **Controllo Output (Relay)**:
    * Sottoscrizione a topic specifici (es. `opta/relay/1`) per ricevere comandi.
    * Comandi `"ON"` e `"OFF"` ricevuti sui topic corrispondenti controllano lo stato dei relay (`D0` - `D3`).

* **Modalità Debug**:

* Includendo `bool DEBUG = true;` all'inizio dello sketch, è possibile abilitare i messaggi di debug sulla console seriale. Questi messaggi forniscono dettagli sullo stato della connessione e sull operazioni MQTT.

## Configurazione

Prima di caricare lo sketch, assicurati di configurare correttamente i seguenti parametri:

### Parametri del Server MQTT

Questi valori devono corrispondere alla configurazione del tuo broker MQTT:

* `mqtt_server`: Indirizzo IP del tuo broker MQTT (es. `"192.168.xxx.xxx"`).
* `mqtt_port`: Porta del broker MQTT (solitamente `1883`).
* `mqtt_client_id`: Un ID univoco per il client MQTT (es. `"OptaClient"`).
* `mqtt_user`: Username per l'autenticazione al broker.
* `mqtt_password`: Password per l'autenticazione al broker.

### Topic MQTT

I topic sono predefiniti nello sketch. Puoi modificarli per adattarli alla tua struttura MQTT:

#### Topic di Pubblicazione (Finder Opta invia dati)

* **Input Digitali**:
    * `opta/input/1/stato` ... `opta/input/7/stato`
* **Input Analogici**:
    * `opta/input/8/stato`
* **Stato Relay**:
    * `opta/relay/1/stato` ... `opta/relay/4/stato`

#### Topic di Sottoscrizione (Finder Opta riceve comandi)

* **Controllo Relay**:
    * `opta/relay/1` ... `opta/relay/4`

### Configurazione Pin

Lo sketch è configurato per utilizzare i seguenti pin del Finder Opta:

* **Input Digitali**: `A0`, `A1`, `A2`, `A3`, `A4`, `A5`, `A6`
* **Input Analogici**: `A7`
* **Output (Relay)**: `D0`, `D1`, `D2`, `D3`

I LED integrati nel Finder Opta (`LED_D0`, `LED_D1`, `LED_D2`) vengono utilizzati come indicatori visivi dello stato della connessione:

* 🟢 `LED_D0`: Indica lo stato della connessione alla **rete Ethernet**. Lampeggia se la connessione fallisce.
* 🟢 `LED_D1`: Indica lo stato della connessione al **server MQTT**.
* 🟢 `LED_D2`: Lampeggia ad ogni ciclo di comunicazione MQTT, indicando l'attività.

## Requisiti

Per compilare e caricare questo sketch, avrai bisogno di:

* Una dispositibo **Finder Opta**.
* La libreria **Ethernet** installata nel tuo IDE Arduino.
* La libreria **PubSubClient** installata nel tuo IDE Arduino.
* L'ambiente di sviluppo **Arduino IDE**.
* Un file `opta_info.h` che definisca le informazioni della scheda (es. `mac_address`).
