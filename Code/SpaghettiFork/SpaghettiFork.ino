#define SW_BUILTIN 9
#define LoopInterval  300
#define CommonSenseSpeed  30
#define LEDC_PWM_Freq 5000
#define LEDC_PWM_Res  8
#define LEDC_Channel  0
#define LEDC_PWM_Pin  6
bool Crazy = 0;
// 장비의 고유 식별부호입니다. 센서와 안경은 동일한 장비식별부호를 가져야 하며, 서로 다른 장비쌍은 다른 장비식별부호를 가져야 합니다.
// https://www.uuidgenerator.net/ 에서 식별부호를 새로 발급할 수 있습니다
#define SERVICE_UUID  "c2e253d3-eacf-4de9-a53c-d1b76f846eaa"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pChar_STCR = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

#define CHAR_UUID_STCR "d0e4ffcf-8a71-44ef-9bb9-af3313f5ff00"
#define CHAR_UUID_SRCT "4cb604bb-256d-4a12-b98e-f3064b0f8dc7"
#define SERVER_NAME    "Geekble_SpaghettiFork"

class ServerCallback: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Connected");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("DisConn.");
    }
};

class Char_RX_Callback : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristicTemp) {
        std::string value = pCharacteristicTemp->getValue();

        if (!value.empty()) {
            Serial.print("RX: ");
            // 첫 번째 문자를 추출하고, 이를 정수로 변환한 다음, bool 타입으로 변환합니다.
            Crazy = (value[0] == 1);
            Serial.print("Crazy value: ");
            Serial.println(Crazy);
        }
    }
};


void setup() {
  Serial.begin(115200);
  pinMode(SW_BUILTIN, INPUT);

  pinMode(LEDC_PWM_Pin, OUTPUT);
  ledcSetup(LEDC_Channel, LEDC_PWM_Freq, LEDC_PWM_Res);
  ledcAttachPin(LEDC_PWM_Pin, LEDC_Channel);
  ledcWrite(LEDC_Channel, 0);

  BLEDevice::init(SERVER_NAME);
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallback());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pChar_STCR = pService->createCharacteristic(
                      CHAR_UUID_STCR,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pChar_STCR->addDescriptor(new BLE2902());

  BLECharacteristic* pChar_SRCT = pService->createCharacteristic(
                      CHAR_UUID_SRCT,
                      BLECharacteristic::PROPERTY_WRITE
                    );
  pChar_SRCT->setCallbacks(new Char_RX_Callback());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  //  Server TX
  if (deviceConnected) {
    //pChar_STCR->setValue(RGB_Color, sizeof(RGB_Color));
    //pChar_STCR->notify();
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
      delay(500); // give the bluetooth stack the chance to get things ready
      pServer->startAdvertising(); // restart advertising
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
      // do stuff here on connecting
      oldDeviceConnected = deviceConnected;
  }
  if (Crazy) {
    ledcWrite(LEDC_Channel, 255);
  } else if (digitalRead(SW_BUILTIN)){
    ledcWrite(LEDC_Channel, 0);
  } else {
    ledcWrite(LEDC_Channel, CommonSenseSpeed);
  }
  delay(LoopInterval);
}