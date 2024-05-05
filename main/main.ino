/* LIBRARY ARDUINO DEFINE  */
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h> /* Download firmware - OTA */
#include <ESP8266HTTPClient.h> /* ESP8266HTTPClient for HTTP */
#include <WiFiClientSecure.h>  /* WiFiClientSecure for HTTPS*/
#include <ArduinoJson.h>       /* ARDUINO JSON*/
#include <EEPROM.h>
#include <time.h>
#include "ESP8266WebServer.h"
#include <Adafruit_GFX.h>	 // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>

/* USER DEFINE  */
#include "src/include/Macro_define.h"         /* DEFINE MACRO */
#include "src/include/DigiCertGlobalRootCA.h" /* DEFINE DigiCertGlobalRootCA */
#include "src/include/bitmap.h" /* DEFINE bitmap */

void setup()
{
  // Initialize LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  // Initialize Serial communication at baud rate 115200
  Serial.begin(115200);
  delay(500);

  // Initialize TFT 1.44 inch
	tft.initR(INITR_144GREENTAB); // Init ST7735R chip, green tab
	Serial.println(F("Initialize TFT 1.44 inch"));
	tft.fillScreen(ST77XX_BLACK);

  sprintf(string_tft, "bangnguyendev");
	testdrawtext(5, 5, string_tft, 1, ST77XX_YELLOW);
  sprintf(string_tft, " -ESP ID: %X", CHIPID);   
  testdrawtext(5, 15, string_tft, 1, ST77XX_YELLOW);
  sprintf(string_tft, " -Firmware: %s", FirmwareVer);
  testdrawtext(5, 25, string_tft, 1, ST77XX_YELLOW);
  sprintf(string_tft, " -Device: %d MHz", ESP.getCpuFreqMHz()); 
  testdrawtext(5, 35, string_tft, 1, ST77XX_YELLOW);

  tft.drawBitmap(32, 64, wifi, 64, 64, ST77XX_CYAN);
  yield();

  Serial.printf("\n\n>>> ESP ID: %X \r\n", CHIPID);                        // Display ESP chip ID
  Serial.printf(">>> Firmware Version: %s \n", FirmwareVer);               // Display current firmware version
  Serial.printf(">>> Device: %d MHz \n", ESP.getCpuFreqMHz());             // Display device CPU frequency
  Serial.printf(">>> Boot Mode: %d \n", ESP.getBootMode());                // Display boot mode
  Serial.printf(">>> Free Sketch Space: %d \n", ESP.getFreeSketchSpace()); // Display free sketch space

  // Initialize EEPROM module
  EEPROM.begin(512);
  Serial.println("\n\n>>>>>>>>>> 1. Read EEPROM \n");

  // Read SSID from EEPROM
  Serial.println("Reading EEPROM ssid ......");
  String esid = "";
  for (int i = START_EEPROM_SSID; i < END_EEPROM_SSID; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.print(">>>>> SSID: ");
  Serial.println(esid);

  // Read password from EEPROM
  Serial.println("Reading EEPROM pass ......");
  String epass = "";
  for (int i = START_EEPROM_PASS; i < END_EEPROM_PASS; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print(">>>>> PASS: ");
  Serial.println(epass);

  // Check if WiFi connection is successful
  Serial.println("\n\n>>>>>>>>>> 2. VerifyConnection_WIFI \n");
  // Check if SSID length is valid
  if (esid.length() <= SIZE_NAME_SSID)
  {
    // Invalid SSID, start SmartConfig process
    Serial.println("Invalid or empty SSID !!!");
    Serial.println("Require using EspTouch App !!!");
    SmartConfigESP();
  }
  else
  {
    // Set WiFi mode to station mode
    WiFi.mode(WIFI_STA);
    // Connect to WiFi network using SSID and password
    WiFi.begin(esid.c_str(), epass.c_str());

    // Check if WiFi connection is successful
    if (VerifyConnection_WIFI())
    {
      // Print WiFi diagnostics
      WiFi.printDiag(Serial);
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      Serial.print("MAC address: ");
      Serial.println(WiFi.macAddress());
      // print your board's IP address:
      IPAddress ip = WiFi.localIP();
      sprintf(string_tft, " -IP: %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
      testdrawtext(5, 45, string_tft, 1, ST77XX_YELLOW);
    }
    else
    {
      // WiFi connection failed, start SmartConfig process
      Serial.println("WL_CONNECT_FAILED !!!");
      Serial.println("Require using EspTouch App !!!");
      SmartConfigESP();
    }
  }

  // Synchronize time useing SNTP. This is necessary to verify that
  // the TLS certificates offered by the server are currently valid.
  /* In cases when NTP is not used, app must set a time manually to check cert validity */
  /* 1. Update time from the server vn.pool.ntp.org */
  Connect_Localtime_NTP();
  /* 2. Check firmware for updates */
  update_FOTA();
  /* 3. Open port 80 Webserver ESP8266 */
  Serial.println("\n>>>>>>>>>> 5. Opening port 80 Webserver ESP8266!");
  server.on("/", HTTP_GET, handleRoot);
  server.on("/led_on", HTTP_GET, ledOn);
  server.on("/led_off", HTTP_GET, ledOff);
  server.on("/handleUpdateFirmware", HTTP_GET, handleUpdateFirmware);
  server.begin();
  Serial.printf("\n>>> Open Webserver port 80 at IP: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  // put your main code here, to run repeatedly:
  // time_t rawtime;
  // struct tm *timeinfo;

  // time(&rawtime);
  // timeinfo = localtime(&rawtime);
  // strftime(buffer_sent_serial, 80, "%H:%M:%S %d-%B-%Y", timeinfo);
  // Serial.printf("Time now: %s \n", buffer_sent_serial);
  server.handleClient(); 
}

bool VerifyConnection_WIFI()
{
  int attempts = 0; // Variable to count connection attempts
  int ledState = LOW;
  while (attempts < 40)
  {
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState); // Bật hoặc tắt LED
    // Loop until the number of attempts exceeds 40
    switch (WiFi.status())
    {
    case WL_CONNECTED:
      Serial.println("WL_CONNECTED");
      // Blink the LED to indicate successful connection
      for (int i = 0; i < 15; i++)
      {
        digitalWrite(LED_BUILTIN, LOW);  // Turn off the LED
        delay(50);                       // Wait for 0.5 seconds
        digitalWrite(LED_BUILTIN, HIGH); // Turn on the LED
        delay(50);                       // Wait for 0.5 seconds
      }
      return true; // Return true to indicate successful connection
    case WL_CONNECT_FAILED:
      Serial.println("WL_CONNECT_FAILED");
      break;
    case WL_CONNECTION_LOST:
      Serial.println("WL_CONNECTION_LOST");
      break;
    case WL_WRONG_PASSWORD:
      Serial.println("WL_WRONG_PASSWORD");
      break;
    case WL_DISCONNECTED:
      Serial.println("WL_DISCONNECTED");
      break;
    default:
      Serial.println("Unknown Status");
      break;
    }
    attempts++;  // Increment the attempts count
    delay(1000); // Wait for 1 second before checking again
  }

  // Connection unsuccessful after too many attempts
  Serial.println("Connect timed out, opening AP");
  return false; // Return false to indicate unsuccessful connection
}

void SmartConfigESP()
{
  // Set WiFi mode to station (connect to an existing network)
  WiFi.mode(WIFI_STA);

  // Start SmartConfig process (allows device to configure WiFi without manual entry)
  WiFi.beginSmartConfig();

  // Print message to serial monitor indicating SmartConfig start
  Serial.println("Begin SmartConfig!!!");

  // Print message to serial monitor indicating wait time
  Serial.println("Wait for 100 seconds");

  // Initialize counter variable for timeout
  // Wait for connection (timeout after 100 seconds)
  const int timeoutSeconds = 100;
  unsigned long startTime = millis();

  // Loop indefinitely
  while (millis() - startTime < timeoutSeconds * 1000)
  {
    // Delay for 1 second
    delay(1000);

    // Check if timeout is reached (100 seconds)
    Serial.printf("Time remaining for connection: %d seconds\n", timeoutSeconds - (millis() - startTime) / 1000);

    // Check if SmartConfig connection is successful
    if (WiFi.smartConfigDone())
    {
      // Retrieve the configured SSID (network name)
      String qsid = WiFi.SSID();

      // Retrieve the configured password
      String qpass = WiFi.psk();

      // Print message to serial monitor indicating SmartConfig success
      Serial.println("SmartConfig successful");

      // Print retrieved SSID to serial monitor
      Serial.printf("SSID: ");
      Serial.println(qsid);

      // Print retrieved password to serial monitor
      Serial.printf("Password: ");
      Serial.println(qpass);

      // (Optional) Activate a signal (bell or return) upon successful connection (commented out)
      // digitalWrite(LED_BUILTIN, ESP_NB_ON);

      // Check if SSID and password have valid lengths
      if (qsid.length() > SIZE_NAME_SSID && qpass.length() >= SIZE_CHAR_PASS)
      {
        // Print message to serial monitor indicating EEPROM clearing
        Serial.println("Clearing EEPROM");

        // Clear the EEPROM memory (used for non-volatile storage)
        for (int i = START_EEPROM_SSID; i < END_EEPROM_PASS; ++i)
        {
          EEPROM.write(i, 0);
        }

        // Print message to serial monitor indicating start of SSID writing to EEPROM
        Serial.println("Writing EEPROM SSID:");

        // Write the SSID to EEPROM (one character at a time)
        for (int i = 0; i < (int)qsid.length(); ++i)
        {
          EEPROM.write(i + START_EEPROM_SSID, qsid[i]);
          Serial.printf("Wrote [%d]: ", i + START_EEPROM_SSID);
          Serial.println(qsid[i]);
        }

        // Print message to serial monitor indicating start of password writing to EEPROM
        Serial.println("Writing EEPROM password:");

        // Write the password to EEPROM (one character at a time, starting at a different address)
        for (int i = 0; i < (int)qpass.length(); ++i)
        {
          EEPROM.write(i + START_EEPROM_PASS, qpass[i]);
          Serial.printf("Wrote [%d]: ", i + START_EEPROM_PASS);
          Serial.println(qpass[i]);
        }

        // Commit changes to EEPROM memory
        EEPROM.commit();

        // (Optional) Deactivate the signal upon successful saving (commented out)
        // digitalWrite(LED_BUILTIN, ESP_NB_OFF);

        // Print message to serial monitor indicating successful saving of ID and password
        Serial.println("EEPROM.commit() =>>> Saved ID & Pass Wifi");

        // Delay for 200 milliseconds
        delay(200);

        // Exit the loop
        return;
      }
    }
  }
  // Print message to serial monitor indicating automatic restart
  Serial.println("Auto restart.");

  // Delay for 2 seconds
  delay(2000);

  // Restart the ESP board
  ESP.restart();
}

void Connect_Localtime_NTP()
{
  configTime(7 * 3600, 0, "vn.pool.ntp.org", "time.nist.gov");
  Serial.println("\n\n>>>>>>>>>> 3. Connect NTP \n");
  Serial.println("Update Time <<< - >>> vn.pool.ntp.org \n");
  while (!time(nullptr))
  {
    Serial.println("\nWaiting for time");
    Serial.print(".");
    delay(500);
    yield(); // disble Soft WDT reset - NodeMCU
  }

  // Initialize counter variable for timeout
  // Wait for connection (timeout after 100 seconds)
  const int timeoutSeconds = 100;
  unsigned long startTime = millis();

  // Loop indefinitely
  while (millis() - startTime < timeoutSeconds * 1000)
  {
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer_year, 80, "%Y", timeinfo);
    timeinfo = localtime(&rawtime);
    Serial.printf("Year: %s \n", buffer_year);
    int time_year = atoi(buffer_year);
    while (time_year > 2023)
    {
      strftime(buffer_sent_serial, 80, "%H:%M:%S %d-%B-%Y", timeinfo);
      Serial.printf("Time now: %s \n", buffer_sent_serial);
      return;
    }
    // Delay for 1 second
    delay(1000);
  }
  // Print message to serial monitor indicating automatic restart
  Serial.println("Auto restart.");

  // Delay for 2 seconds
  delay(2000);

  // Restart the ESP board
  ESP.restart();
}

void update_FOTA()
{
  Serial.println("\n>>>>>>>>>> 4. FOTA Workflows \n"); // Indicate the start of FOTA update process
  Serial.println("Check firmware for updates.");       // Prompt to check for firmware updates

  Serial.println("Checking for updates"); // Display message indicating checking for updates

  /* Display loading... */
  Serial.printf(">>> ESP ID: %X \r\n", CHIPID);                            // Display ESP chip ID
  Serial.printf(">>> Firmware Version: %s \n", FirmwareVer);               // Display current firmware version
  Serial.printf(">>> Free Sketch Space: %d \n", ESP.getFreeSketchSpace()); // Display free sketch space

  // Secure for HTTPS connection
  // 1. Using certificate
  // client_HTTPS.setTrustAnchors(&cert);
  // 2. Using fingerprint
  client_HTTPS.setFingerprint(fingerprint);

  // Not secure
  // client_HTTPS.setInsecure();
  if (!client_HTTPS.connect(host, httpsPort))
  {
    Serial.println(">>> raw.githubusercontent.com - Connection failed"); // Display connection failure message
    Serial.printf(">>> client_HTTPS.connect(host, httpsPort) is failed \n");
    // Print SSL error code
    // Serial.printf("SSL error code: %s\n", client_HTTPS.getLastSSLError());
    return;
  }

  client_HTTPS.print(String("GET ") + URL_fw_Version + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" +
                     "User-Agent: ESP8266\r\n" +
                     "Connection: close\r\n\r\n");

  while (client_HTTPS.connected())
  {
    String payload = client_HTTPS.readString(); // Get the request response payload
    // Serial.println(payload);                    // Indicate that headers have been received

    // get the payload JSON
    // Tìm vị trí của dấu mở ngoặc nhọn đầu tiên
    int jsonStart = payload.indexOf("{");
    if (jsonStart != -1)
    {
      // Tìm thấy đoạn JSON trong chuỗi text
      String jsonText = payload.substring(jsonStart);
      Serial.println("JSON output:");
      Serial.println(jsonText);

      JsonDocument jsonBuffer;
      DeserializationError error = deserializeJson(jsonBuffer, jsonText);

      if (error)
      {
        Serial.print(F("deserializeJson() failed with code ")); // Display JSON deserialization failure message
        Serial.println(error.c_str());
        return;
      }
      String author_prod = jsonBuffer["author"];
      String version_prod = jsonBuffer["main"]["version"];

      if (version_prod.equals(FirmwareVer))
      {
        Serial.printf(">>> Board__Firmware Version: %s \n", FirmwareVer);  // Display Board current firmware version
        Serial.printf(">>> Server_Firmware Version: %s \n", version_prod); // Display Server current firmware version
        Serial.println(">>> Device already on latest firmware version");   // Display message indicating already on latest firmware version
      }
      else
      {
        Serial.printf(">>> New firmware detected: "); // Display message indicating new firmware detected
        Serial.print(FirmwareVer);
        Serial.print(" -----> ");
        Serial.println(version_prod);
        // The line below is optional. It can be used to blink the LED on the board during flashing
        // The LED will be on during download of one buffer of data from the network. The LED will
        // be off during writing that buffer to flash
        // On a good connection the LED should flash regularly. On a bad connection the LED will be
        // on much longer than it will be off. Other pins than LED_BUILTIN may be used. The second
        // value is used to put the LED on. If the LED is on with HIGH, that value should be passed
        ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

        // Add optional callback notifiers
        ESPhttpUpdate.onStart(update_started);
        ESPhttpUpdate.onEnd(update_finished);
        ESPhttpUpdate.onProgress(update_progress);
        ESPhttpUpdate.onError(update_error);

        t_httpUpdate_return ret = ESPhttpUpdate.update(client_HTTPS, URL_fw_Bin);

        switch (ret)
        {
        case HTTP_UPDATE_FAILED:
          Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str()); // Display HTTP update failed message
          Serial.println(">>> Server overloaded or unavailable...");                                                                      // Display server unavailability message
          Serial.println(">>> Or your device is not permitted to update on the system...");                                               // Display device update permission message
          Serial.println(">>> Please check for updates at another time...");                                                              // Prompt to check for updates later
          Serial.printf(">>> Current version is %s \n", FirmwareVer);                                                                     // Display current firmware version
          Serial.println("> Skip updated...ERR");                                                                                         // Display skipping update message
          delay(2000);
          break;

        case HTTP_UPDATE_NO_UPDATES:
          Serial.println("HTTP_UPDATE_NO_UPDATES");                 // Display message indicating no updates available
          Serial.println(">>> The current version is the latest."); // Display message indicating already on latest version
          delay(1500);
          break;

        case HTTP_UPDATE_OK:
          Serial.println("HTTP_UPDATE_OK"); // Display message indicating update successful
          break;
        }
      }
      Serial.println("\n<<<<<<<<<< Done Check FOTA >>>>>>>>>>\n"); // Indicate completion of FOTA update process
    }
    else
    {
      Serial.println("JSON not found in payload!");
      return;
    }
  }
}

String convertToFormattedString(uint32_t number) {
    char buffer[12]; // Đủ lớn để chứa chuỗi định dạng
    sprintf(buffer, "%02d %02d %02d %02d %02d",
            (number / 10000000) % 100, (number / 100000) % 100, (number / 1000) % 100,
            (number / 10) % 100, number % 100);
    return String(buffer);
}

void handleRoot() {
  String message = "<!DOCTYPE html>";
  message += "<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>ESP8266 WebServer</title></head><body>";
  message += "<h1 style=\"text-align: center;\">ESP8266 WebServer</h1>";
  message += "<div style=\"text-align: center;\">";
  message += "<p>Author: " + Author + "</p>";
  message += "<p>ESP ID: " + String(CHIPID, HEX) + "</p>";
  message += "<p>ESP MAC: " + WiFi.macAddress() + "</p>";
  message += "<p>Version Firmware: " + FirmwareVer + "</p>";
  // Hiển thị trạng thái thao tác (nếu có)
  if (server.hasArg("message")) {
    message += "<p>" + server.arg("message") + "</p>";
  }
  message += ledOnButton;
  message += ledOffButton;
  message += "<br>";
  message += updateButton;
  message += "<br>";
  message += "<p>RANDOM_RNG: " + convertToFormattedString(RANDOM_REG32) + "</p>";
  message += "</div></body></html>";
  server.send(200, "text/html", message);
}

void ledOn() {
  Serial.println(">>>>>>>>>> ledOnButton ......");
  digitalWrite(LED_BUILTIN, LOW); 
  server.sendHeader("Location", "/?message=LED+is+now+on");
  server.send(303);
}

void ledOff() {
  Serial.println(">>>>>>>>>> ledOffButton ......");
  digitalWrite(LED_BUILTIN, HIGH);
  server.sendHeader("Location", "/?message=LED+is+now+off");
  server.send(303);
}

void handleUpdateFirmware() {
  Serial.println(">>>>>>>>>> handleUpdateFirmware ......");
  server.sendHeader("Location", "/?message=Check+Firmware+update");
  server.send(303);
  update_FOTA();
}

void testdrawtext(int x, int y, char * text, int z, uint16_t color)
{
	tft.setCursor(x, y);
	tft.setTextSize(z);
	tft.setTextColor(color);
	tft.setTextWrap(true);
	tft.print(text);
}
