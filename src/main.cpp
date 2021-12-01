/*
  Titre      : Controle de luminosité
  Auteur     : Yvan Tankeu
  Date       : 26/11/2021
  Description: lecture d'intensité lumineuse et actionnage de lumière
  Version    : 0.0.1
  bon à savoir     : J'ai réutilisé le code D'André afin d'y ajuster pour le cas écheant
*/

#include <Arduino.h>
#include "WIFIConnector_MKR1000.h"
#include "MQTTConnector.h"

/*
 declaration de la broche digital 5 et 6
*/
#define ledBleue 5
#define ledRouge 6

// Fonction de qui permet d'allumer et d'éteindre les led
#define ledBleueOn digitalWrite(ledBleue, HIGH)
#define ledBleueOff digitalWrite(ledBleue, LOW)

#define ledRougeOn digitalWrite(ledRouge, HIGH)
#define ledRougeOff digitalWrite(ledRouge, LOW)

const int ANALOG_PIN = A1; // Utilisation de la broche A1 pour lecture analogue

// Variable de controle générales tel les délais, etc...

const int MS_DELAI = 5000; // Nombre de milliseconde de délais

int croisement(0), route(0), analogueValue(0);

/*
  Variables pour obtenir une valeur de tension a partir du résultat du convertisseur
  analogique digitale CAD, de 0 à 1023 en 0 à 3.3V.  A ajuster selon la tension maximale
  permise par le microncontrolleur et la resolution de la broche 8,10,12,...  bits
*/

const float PIN_BASE_MAX_VOLTAGE = 3.3;    // Tension de base maximale pour la broche analogue du uC utilisé
const float MIN_VOLTAGE = 0;    // Tension de base minimal pour la broche analogue du uC utilisé
const int PIN_ANALOG_MAX_VALUE = 1023; // Valeur maximale de la lecture de la broche analogue
const int PIN_ANALOG_MIN_VALUE = 0; // Valeur minimale de la lecture de la broche analogue

float pinVoltage = 0;                  // Variable pour la transformation de la lecture analogue a une tension

// La fonction setup sert, entre autre chose, a configurer les broches du uC

void setup()
{

  Serial.begin(9600); // Activation du port série pour affichage dans le moniteur série

  wifiConnect(); //Branchement au réseau WIFI
  MQTTConnect(); //Branchement au broker MQTT

  pinMode(ANALOG_PIN, INPUT); // Pour une bonne lecture, la broche analogique doit être placé en mode entré explicitment

  // Init des broches 13 et 12 en mode sortie
  pinMode(ledBleue, OUTPUT);
  pinMode(ledRouge, OUTPUT);
}

// Boucle infinie qui permet au uC de faire une ou plusieurs taches sans arrêt

void loop()
{
  float valLumens(0);
  analogueValue = analogRead(ANALOG_PIN); // lecture de la valeur de la broche A1

  // En utilisant la fonction map(), on pourra mapper la valeur lue par la broche 
  //analague d’une plage de 0 à 1023 à la plage de la tension de 0 à 3.3v, 
  pinVoltage = map(analogueValue, PIN_ANALOG_MIN_VALUE, PIN_ANALOG_MAX_VALUE, 
  MIN_VOLTAGE, PIN_BASE_MAX_VOLTAGE);

  valLumens = pinVoltage;

  Serial.print("La valeur obtenue par la broche analogue est ");
  Serial.println(analogueValue);

  Serial.print("La tension obtenue due convertisseur CAD ");
  Serial.println(pinVoltage);

  // on utilisera la valeur lue la broche analogue comme donnée de decision
    // activer les feux de routes si l'intensité est plus petite que 400
  if (valLumens  <=  300 )
  {
    ledRougeOn;
    ledBleueOff;
    //delay(1000); // Wait for 1000 millisecond(s)
    route = valLumens;
  }
  // activer les feux de routes si l'intensité est plus grande que 400
  else if (valLumens >= 400)
  {
    ledRougeOff;
    ledBleueOn;
    //delay(100); // Wait for 1000 millisecond(s)

    croisement = valLumens;
  }
  else
  {
    ledRougeOff;
    ledBleueOff;
  }
  
  appendPayload("Lumens", valLumens); //Ajout de la donnée température au message MQTT
  sendPayload();                                  //Envoie du message via le protocole MQTT

  appendPayload("Feu de croisement Activé", croisement);
  sendPayload();

  appendPayload("Feu de route activé", route);
  sendPayload();

  delay(MS_DELAI); // Délai de sorte a ce qu'on puisse lire les valeurs et ralentir le uC
                   // Note: l'utilisation d'un délai est généralement une mauvaise pratique mais utilisable dans le cas de ce démo
}