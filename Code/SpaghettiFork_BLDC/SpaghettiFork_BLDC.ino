// CommonSenseSpeed의 값을 조절하여 포크의 회전속도를 조절합니다
// 255에 가까울수록 느려지고, 0에 가까울수록 빨라집니다
#define CommonSenseSpeed  0

#define LoopInterval      150   // 루프 간격을 정의하는 변수입니다. 루프의 주기를 결정합니다.
#define CrazySpeed        0     // "Crazy" 모드일 때의 회전 속도를 결정하는 변수입니다.
#define StopSpeed         255   // 포크를 정지시킬 때의 회전 속도를 결정하는 변수입니다.
#define LEDC_PWM_Freq     4000  // LEDC PWM의 주파수를 설정하는 변수입니다.
#define LEDC_PWM_Res      8     // LEDC PWM의 해상도를 설정하는 변수입니다.
#define LEDC_Channel      0     // LEDC 채널을 설정하는 변수입니다.
#define LEDC_PWM_Pin      6     // LEDC PWM을 사용할 핀을 설정하는 변수입니다.

// "Crazy" 모드 여부를 나타내는 변수입니다.
bool Crazy = 0;

// BLE 디바이스의 서비스 UUID를 설정하는 변수입니다.
#define SERVICE_UUID  "c2e253d3-eacf-4de9-a53c-d1b76f846eaa"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL; // BLE 서버 객체
BLECharacteristic* pChar_STCR = NULL; // BLE 특성 객체
bool deviceConnected = false; // 디바이스 연결 여부
bool oldDeviceConnected = false; // 이전 연결 여부
uint32_t value = 0; // 포크의 회전 속도 값

#define CHAR_UUID_STCR "d0e4ffcf-8a71-44ef-9bb9-af3313f5ff00"
#define CHAR_UUID_SRCT "4cb604bb-256d-4a12-b98e-f3064b0f8dc7"
#define SERVER_NAME    "Geekble_SpaghettiFork"
/*
// BLE 서버의 콜백 클래스
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

// 수신된 BLE 특성 값을 처리하는 콜백 클래스
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

*/
void setup() {
  Serial.begin(115200); // 시리얼 통신 초기화
  pinMode(SW_BUILTIN, INPUT); // 내장 버튼을 입력으로 설정
  pinMode(LEDC_PWM_Pin, OUTPUT); // LEDC PWM 핀을 출력으로 설정
  digitalWrite(LEDC_PWM_Pin, HIGH); // LEDC PWM 핀 활성화
  ledcSetup(LEDC_Channel, LEDC_PWM_Freq, LEDC_PWM_Res); // LEDC 설정
  ledcAttachPin(LEDC_PWM_Pin, LEDC_Channel);
  ledcWrite(LEDC_Channel, StopSpeed);

  BLEDevice::init(SERVER_NAME); // BLE 초기화
  pServer = BLEDevice::createServer();  // BLE 서버 생성
  pServer->setCallbacks(new ServerCallback());

  BLEService *pService = pServer->createService(SERVICE_UUID);  // BLE 서비스 생성

  // BLE 특성 생성
  pChar_STCR = pService->createCharacteristic(
                      CHAR_UUID_STCR,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pChar_STCR->addDescriptor(new BLE2902()); // BLE 디스크립터  추가

  // BLE 특성 생성
  BLECharacteristic* pChar_SRCT = pService->createCharacteristic(
                      CHAR_UUID_SRCT,
                      BLECharacteristic::PROPERTY_WRITE
                    );
  pChar_SRCT->setCallbacks(new Char_RX_Callback()); // BLE 콜백 설정

  pService->start();  // BLE 서비스 시작

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising(); // 광고 시작
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  if (deviceConnected) {  // 서버TX
    //pChar_STCR->setValue(RGB_Color, sizeof(RGB_Color));
    //pChar_STCR->notify();
  }
  if (!deviceConnected && oldDeviceConnected) { // 연결 해제시
      delay(500); // 인터벌
      pServer->startAdvertising(); // 광고 재시작
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) { // 연결
      // do stuff here on connecting
      oldDeviceConnected = deviceConnected;
  }
  if (Crazy) {  // Crazy 모드인 경우
    ledcWrite(LEDC_Channel, CrazySpeed);
  } else if (digitalRead(SW_BUILTIN)){  // 내장 버튼이 눌려 있는 경우
    ledcWrite(LEDC_Channel, StopSpeed);
  } else {  // 일반적인 경우
    ledcWrite(LEDC_Channel, CommonSenseSpeed);
  }
  delay(LoopInterval);  // 루프 간격 동안 대기
}