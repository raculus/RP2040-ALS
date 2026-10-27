#include <Adafruit_TinyUSB.h>

#define TEMT6000_PIN 26
// 보정용 변수
const int ADC_MIN = 60;      // 암실(불 껐을 때) 측정 ADC 값 (0 lx 기준)
const int ADC_REF = 140;     // 스마트폰 실제 raw_adc 측정값
const float LUX_REF = 160.0f; // 기준 스마트폰 조도 값
// --------------------------------------------------------------------+
// HID Sensor (Ambient Light) Report Descriptor
// --------------------------------------------------------------------+
#define SEL(lo, hi) 0x0A, lo, hi   // selector usage

uint8_t const desc_hid_report[] = {
  0x05, 0x20,              // Usage Page (Sensors)
  0x09, 0x01,              // Usage (Sensor)
  0xA1, 0x01,              // Collection (Application)
  0x85, 0x01,              //   Report ID (1)

  0x09, 0x41,              //   Usage (Light: Ambient Light)
  0xA1, 0x00,              //   Collection (Physical)

    // ---------- Feature: Connection Type ----------
    0x0A, 0x09, 0x03,      //     Usage (Property: Sensor Connection Type)
    0x15, 0x00, 0x25, 0x02, 0x75, 0x08, 0x95, 0x01,
    0xA1, 0x02,            //     Collection (Logical)
      SEL(0x30, 0x08),     //       Integrated
      SEL(0x31, 0x08),     //       Attached
      SEL(0x32, 0x08),     //       External
      0xB1, 0x00,          //       Feature (Data, Array, Abs)
    0xC0,

    // ---------- Feature: Reporting State ----------
    0x0A, 0x16, 0x03,      //     Usage (Property: Reporting State)
    0x15, 0x00, 0x25, 0x05, 0x75, 0x08, 0x95, 0x01,
    0xA1, 0x02,
      SEL(0x40, 0x08),     //       No Events
      SEL(0x41, 0x08),     //       All Events
      SEL(0x42, 0x08),     //       Threshold Events
      SEL(0x43, 0x08),     //       No Events Wake
      SEL(0x44, 0x08),     //       All Events Wake
      SEL(0x45, 0x08),     //       Threshold Events Wake
      0xB1, 0x00,
    0xC0,

    // ---------- Feature: Power State ----------
    0x0A, 0x19, 0x03,      //     Usage (Property: Power State)
    0x15, 0x00, 0x25, 0x05, 0x75, 0x08, 0x95, 0x01,
    0xA1, 0x02,
      SEL(0x50, 0x08),     //       Undefined
      SEL(0x51, 0x08),     //       D0 Full Power
      SEL(0x52, 0x08),     //       D1
      SEL(0x53, 0x08),     //       D2
      SEL(0x54, 0x08),     //       D3
      SEL(0x55, 0x08),     //       D4 Power Off
      0xB1, 0x00,
    0xC0,

    // ---------- Feature: Sensor State ----------
    0x0A, 0x01, 0x02,      //     Usage (State)
    0x15, 0x00, 0x25, 0x06, 0x75, 0x08, 0x95, 0x01,
    0xA1, 0x02,
      SEL(0x00, 0x08),     //       Undefined
      SEL(0x01, 0x08),     //       Ready
      SEL(0x02, 0x08),     //       Not Available
      SEL(0x03, 0x08),     //       No Data
      SEL(0x04, 0x08),     //       Initializing
      SEL(0x05, 0x08),     //       Access Denied
      SEL(0x06, 0x08),     //       Error
      0xB1, 0x00,
    0xC0,

    // ---------- Feature: Report Interval (ms, 32bit) ----------
    0x0A, 0x0E, 0x03,      //     Usage (Property: Report Interval)
    0x15, 0x00,
    0x27, 0xFF, 0xFF, 0xFF, 0x7F,
    0x66, 0x01, 0x10,      //     Unit (Seconds)
    0x55, 0x0D,            //     Unit Exponent (-3) -> ms
    0x75, 0x20, 0x95, 0x01,
    0xB1, 0x02,            //     Feature (Data, Var, Abs)
    0x66, 0x00, 0x00,      //     Unit 해제
    0x55, 0x00,            //     Exponent 해제

    // ---------- Feature: Illuminance Change Sensitivity (Abs) ----------
    0x0A, 0xD1, 0x14,
    0x15, 0x00,
    0x27, 0xFF, 0xFF, 0x00, 0x00,
    0x75, 0x10, 0x95, 0x01,
    0xB1, 0x02,

    // ---------- Input: Sensor State ----------
    0x0A, 0x01, 0x02,
    0x15, 0x00, 0x25, 0x06, 0x75, 0x08, 0x95, 0x01,
    0xA1, 0x02,
      SEL(0x00, 0x08),
      SEL(0x01, 0x08),
      SEL(0x02, 0x08),
      SEL(0x03, 0x08),
      SEL(0x04, 0x08),
      SEL(0x05, 0x08),
      SEL(0x06, 0x08),
      0x81, 0x00,          //       Input (Data, Array, Abs)
    0xC0,

    // ---------- Input: Sensor Event ----------
    0x0A, 0x02, 0x02,      //     Usage (Event)
    0x15, 0x00, 0x25, 0x05, 0x75, 0x08, 0x95, 0x01,
    0xA1, 0x02,
      SEL(0x10, 0x08),     //       Unknown
      SEL(0x11, 0x08),     //       State Changed
      SEL(0x12, 0x08),     //       Property Changed
      SEL(0x13, 0x08),     //       Data Updated
      SEL(0x14, 0x08),     //       Poll Response
      SEL(0x15, 0x08),     //       Change Sensitivity
      0x81, 0x00,
    0xC0,

    // ---------- Input: Illuminance (lux) ----------
    0x0A, 0xD1, 0x04,      //     Usage (Data Field: Illuminance)
    0x15, 0x00,
    0x27, 0xFF, 0xFF, 0x00, 0x00,
    0x75, 0x10, 0x95, 0x01,
    0x81, 0x02,            //     Input (Data, Var, Abs)

  0xC0,                    //   End Collection (Physical)
  0xC0                     // End Collection (Application)
};

// --------------------------------------------------------------------+
// 구조체 (설명자의 필드 순서와 정확히 일치, Report ID 제외)
// --------------------------------------------------------------------+
typedef struct __attribute__((packed)) {
  uint8_t  connection_type;   // 1: Attached
  uint8_t  reporting_state;   // 1: All Events
  uint8_t  power_state;       // 1: D0 Full Power
  uint8_t  sensor_state;      // 1: Ready
  uint32_t report_interval;   // ms
  uint16_t sensitivity;       // lux
} als_feature_report_t;

typedef struct __attribute__((packed)) {
  uint8_t  sensor_state;      // 1: Ready
  uint8_t  sensor_event;      // 3: Data Updated
  uint16_t lux;
} als_input_report_t;

static als_feature_report_t feature_rep = { 1, 1, 1, 1, 500, 1 };
static als_input_report_t   input_rep   = { 1, 3, 0 };

Adafruit_USBD_HID usb_hid;
uint32_t last_report_ms = 0;

// --------------------------------------------------------------------+
//    GET / SET REPORT 콜백
//    TinyUSB가 Report ID를 자동 처리하므로 buffer에는 데이터만 넣습니다.
// --------------------------------------------------------------------+
uint16_t get_report_callback(uint8_t report_id, hid_report_type_t report_type,
                             uint8_t* buffer, uint16_t reqlen) {
  if (report_type == HID_REPORT_TYPE_FEATURE) {
    uint16_t len = sizeof(feature_rep);
    if (reqlen < len) len = reqlen;
    memcpy(buffer, &feature_rep, len);
    return len;
  }
  if (report_type == HID_REPORT_TYPE_INPUT) {
    uint16_t len = sizeof(input_rep);
    if (reqlen < len) len = reqlen;
    memcpy(buffer, &input_rep, len);
    return len;
  }
  return 0;
}

void set_report_callback(uint8_t report_id, hid_report_type_t report_type,
                         uint8_t const* buffer, uint16_t bufsize) {
  if (report_type == HID_REPORT_TYPE_FEATURE && bufsize > 0) {
    uint16_t len = sizeof(feature_rep);
    if (bufsize < len) len = bufsize;
    memcpy(&feature_rep, buffer, len);
  }
}

// --------------------------------------------------------------------+
// Setup & Loop
// --------------------------------------------------------------------+
void setup() {
  // TEMT6000 전원 (GPIO로 직접 공급)
  pinMode(27, OUTPUT);
  digitalWrite(27, LOW);   // GND
  pinMode(28, OUTPUT);
  digitalWrite(28, HIGH);  // VCC

  analogReadResolution(12);

  // Windows의 실패 캐시를 피하기 위해 새 PID/문자열 사용 (begin 전에 호출)
  TinyUSBDevice.setID(0x239A, 0x8F01);
  TinyUSBDevice.setManufacturerDescriptor("DIY");
  TinyUSBDevice.setProductDescriptor("RP2040 ALS");

  usb_hid.setPollInterval(5);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.setReportCallback(get_report_callback, set_report_callback);
  usb_hid.begin();

  while (!TinyUSBDevice.mounted()) {
    delay(10);
  }
}

void loop() {
  #ifdef TUSB_ADDED_FUNCS
    TinyUSBDevice.task();
  #endif

  uint32_t now = millis();
  uint32_t interval = (feature_rep.report_interval == 0) ? 500 : feature_rep.report_interval;

  if (now - last_report_ms >= interval) {
    last_report_ms = now;

    // reporting_state: 0 = No Events, 1 = All Events (selector 인덱스)
    if (usb_hid.ready() && feature_rep.reporting_state != 0) {
      int raw_adc = analogRead(TEMT6000_PIN);

      int net_adc = raw_adc - ADC_MIN;
      if (net_adc < 0) net_adc = 0;

      // 기울기(Gain) = LUX_REF / (ADC_REF - ADC_MIN)
      float scale_factor = LUX_REF / (float)(ADC_REF - ADC_MIN);
      float lux_float = (float)net_adc * scale_factor;

      // 상한선 Capping (필요에 따른 최대 범위 제한)
      if (lux_float > 1000.0f) lux_float = 1000.0f;

      uint16_t lux = (uint16_t)lux_float;

      input_rep.sensor_state = 1;  // Ready
      input_rep.sensor_event = 3;  // Data Updated
      input_rep.lux          = lux;

      usb_hid.sendReport(1, &input_rep, sizeof(input_rep));
    }
  }
}