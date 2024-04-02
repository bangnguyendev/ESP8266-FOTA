/*                                                                                                                                                               
      _____          ____  _____   ______         _____           _____   ______         _____     ____   ____  _____      _____      ______  _____   ______   
 ___|\     \    ____|\   \|\    \ |\     \    ___|\    \         |\    \ |\     \    ___|\    \   |    | |    ||\    \    /    /| ___|\     \|\    \ |\     \  
|    |\     \  /    /\    \\\    \| \     \  /    /\    \         \\    \| \     \  /    /\    \  |    | |    || \    \  /    / ||     \     \\\    \| \     \ 
|    | |     ||    |  |    |\|    \  \     ||    |  |____|         \|    \  \     ||    |  |____| |    | |    ||  \____\/    /  /|     ,_____/|\|    \  \     |
|    | /_ _ / |    |__|    | |     \  |    ||    |    ____          |     \  |    ||    |    ____ |    | |    | \ |    /    /  / |     \--'\_|/ |     \  |    |
|    |\    \  |    .--.    | |      \ |    ||    |   |    |         |      \ |    ||    |   |    ||    | |    |  \|___/    /  /  |     /___/|   |      \ |    |
|    | |    | |    |  |    | |    |\ \|    ||    |   |_,  |         |    |\ \|    ||    |   |_,  ||    | |    |      /    /  /   |     \____|\  |    |\ \|    |
|____|/____/| |____|  |____| |____||\_____/||\ ___\___/  /|         |____||\_____/||\ ___\___/  /||\___\_|____|     /____/  /    |____ '     /| |____||\_____/|
|    /     || |    |  |    | |    |/ \|   ||| |   /____ / |         |    |/ \|   ||| |   /____ / || |    |    |    |`    | /     |    /_____/ | |    |/ \|   ||
|____|_____|/ |____|  |____| |____|   |___|/ \|___|    | /          |____|   |___|/ \|___|    | /  \|____|____|    |_____|/      |____|     | / |____|   |___|/
  \(    )/      \(      )/     \(       )/     \( |____|/             \(       )/     \( |____|/      \(   )/         )/           \( |_____|/    \(       )/  
   '    '        '      '       '       '       '   )/                 '       '       '   )/          '   '          '             '    )/        '       '   
                                                    '                                      '                                             '                     
*/

/* Cập nhật OTA */

const String FirmwareVer={"v1.0.0"}; 

#define URL_fw_Version "/bangnguyendev/XXXXXXX/master/include/Info_prod.json"
#define URL_fw_Bin "https://raw.githubusercontent.com/bangnguyendev/XXXXXXX/master/build/main.ino.bin"

const char* host = "raw.githubusercontent.com";
const int httpsPort = 443;

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

/* PIN kết nối với chuông kêu */
#define PIN_signal_Bell D4
/* PIN kết nối với nút nhấn MODE */
#define Button_Mode D1

/* Define By User NDB */
#define ESP_NB_ZERO 0
#define ESP_NB_ONE 1
#define ESP_NB_OFF 0
#define ESP_NB_ON 1

/* Cài đặt dạng kết nối nút nhấn PULL_UP OR PULL_DOWN */
#define PULLUP_PULLDOWN 1 // PULL DOWN


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



/* Khai báo cấu hình cho wifi */
const char *ssid = "BV Public";
const char *passphrase = "banviencorp";
WiFiClient client_wifi;

extern void setup();
extern void loop();
extern void update_FOTA();
extern void smartConfig_ndb();
extern bool bool_Test_Wifi();
