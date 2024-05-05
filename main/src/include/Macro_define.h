/* Cập nhật OTA */

const String FirmwareVer={"v1.0.0"}; 
const String Author={"bangnguyendev"}; 

#define Branch_git "TFT_1.44_SPI"
#define URL_fw_Version "/bangnguyendev/ESP8266-FOTA/" Branch_git "/main/src/include/Info_prod.json"
#define URL_fw_Bin "https://raw.githubusercontent.com/bangnguyendev/ESP8266-FOTA/" Branch_git "/build/main.ino.bin"

const char* host = "raw.githubusercontent.com";
const int httpsPort = 443;

WiFiClient client_HTTP; // WiFiClient for HTTP
WiFiClientSecure client_HTTPS; // WiFiClientSecure for HTTPS

ESP8266WebServer server(80);
 
String ledOnButton = "<a href=\"/led_on\"><button style=\"padding: 10px; background-color: #4CAF50; color: white; border: none; text-decoration: none; display: inline-block; font-size: 16px;\">Turn LED On</button></a>";
String ledOffButton = "<a href=\"/led_off\"><button style=\"padding: 10px; background-color: #f44336; color: white; border: none; text-decoration: none; display: inline-block; font-size: 16px;\">Turn LED Off</button></a>";
String updateButton = "<a href=\"/handleUpdateFirmware\"><button style=\"padding: 10px; background-color: #008CBA; color: white; border: none; text-decoration: none; display: inline-block; font-size: 16px;\">Update Firmware</button></a>";


//UPDATER
String last_error;
String update_status;
float progress_percent; 
void update_started() {
    Serial.println("CALLBACK:  HTTP update process started");
    Serial.println("Downloading... ");
}
void update_finished() {
    Serial.println("CALLBACK:  HTTP update process finished");
}
void update_progress(int cur, int total) {
    progress_percent = float((float)cur/(float)total)*100;
    Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes... -[ %0.1f %% ]- \n", cur, total, progress_percent);
}
void update_error(int err) {
    Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}
//UPDATER

/* PIN kết nối với chuông/led LED_BUILTIN D4 */ 

/* PIN kết nối với nút nhấn MODE */
#define Button_Mode D1

#define SIZE_NAME_SSID 0
#define SIZE_CHAR_PASS 8

// Definition for the EEPROM storage area of SSID, starting from address 0
#define START_EEPROM_SSID 0

// Definition for the EEPROM storage area of SSID, ending at address 40
#define END_EEPROM_SSID 40

// Definition for the EEPROM storage area of password, starting from address 40
#define START_EEPROM_PASS 40

// Definition for the EEPROM storage area of password, ending at address 100
#define END_EEPROM_PASS 100

// time NTP
char buffer_sent_serial[80];
char buffer_year[80];

/*   TFT 1.44 inch   */
//GPIO Wemos D1 mini
#define TFT_DC D4
#define TFT_RST D3

//SPI interface - none using MISO
#define TFT_CS D8 // CS
#define TFT_MOSI D7 // Data out
#define TFT_SCLK D5 // Clock out

// For ST7735-based displays, we will use this call
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
// yield(); // https://forum.arduino.cc/t/soft-wdt-reset-nodemcu/425567/2
char string_tft[50] = "";
/*  End TFT 1.44 inch   */

extern void setup();
extern void loop();
extern void SmartConfigESP();
extern bool VerifyConnection_WIFI();
extern void Connect_Localtime_NTP();
extern void update_FOTA();
extern void handleRoot();
extern void ledOn();
extern void ledOff();
extern void handleUpdateFirmware();
extern void testdrawtext(int x, int y, char * text, int z, uint16_t color);
