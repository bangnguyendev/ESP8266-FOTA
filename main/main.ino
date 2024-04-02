/* LIBRARY ARDUINO DEFINE  */
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h> /* Get data Weather - http */
#include <ESP8266httpUpdate.h> /* Updated OTA */
#include <WiFiClientSecure.h>

#include <time.h>
#include <EEPROM.h>
#include <ArduinoJson.h> /* ARDUINOJSON_VERSION "6.x.x" */

/* USER DEFINE  */
#include "../../../ESP8266-FOTA/include/Macro_define.h"	  /* DEFINE MACRO */
#include "../../../ESP8266-FOTA/include/DigiCertGlobalRootCA.h" /* DEFINE DigiCertGlobalRootCA */

void setup() {
  // Initialize Serial communication at baud rate 115200
  Serial.begin(115200);
  
  // Initialize EEPROM module
  EEPROM.begin(512);
  Serial.println("\n\n>>>>>>>>>> Read EEPROM \n");

  // Read WiFi credentials from EEPROM
  Serial.println(">>>>>>>>>> READ SSID PASS EEROM  \n");

  // Read SSID from EEPROM
  Serial.println("Reading EEPROM ssid ......");
  String esid = "";
  for (int i = START_EEPROM_SSID; i < END_EEPROM_SSID; ++i) {
    esid += char(EEPROM.read(i));
  }
  Serial.print(">>>>> SSID: ");
  Serial.println(esid);

  // Read password from EEPROM
  Serial.println("Reading EEPROM pass ......");
  String epass = "";
  for (int i = START_EEPROM_PASS; i < END_EEPROM_PASS; ++i) {
    epass += char(EEPROM.read(i));
  }
  Serial.print(">>>>> PASS: ");
  Serial.println(epass);

  // Check if SSID length is greater than 0
  if (esid.length() > SIZE_NAME_SSID) {
    // Connect to WiFi network using SSID and password
    WiFi.mode(WIFI_STA);
    WiFi.begin(esid.c_str(), epass.c_str());

    // Check if WiFi connection is successful
    if (bool_Test_Wifi()) {
      Serial.println("");
      Serial.println("Wifi Connection Successful!!!");
    } else {
      // WiFi connection failed, start SmartConfig process
      Serial.println("WL_CONNECT_FAILED !!!");
      Serial.println("Require using EspTouch App !!!");
      smartConfig_ndb();
    }
  } else {
    // Invalid SSID, start SmartConfig process
    Serial.println("Invalid or empty SSID !!!");
    Serial.println("Require using EspTouch App !!!");
    smartConfig_ndb();
  }

  // Set WiFi mode to station mode
  WiFi.mode(WIFI_STA);

  // Print WiFi diagnostics
  Serial.println("");
  WiFi.printDiag(Serial);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("\n");

  // Check for firmware updates
  update_FOTA();
}


void loop() {
  // put your main code here, to run repeatedly:

}

void update_FOTA() {
    Serial.println(">>>>>>>>>>> Update FOTA \n"); // Indicate the start of FOTA update process
    Serial.println("Check firmware for updates."); // Prompt to check for firmware updates

    Serial.println("Checking for updates"); // Display message indicating checking for updates

    /* Display loading... */
    
    Serial.println("...");
    Serial.printf(">>> Device: %d MHz \n", ESP.getCpuFreqMHz()); // Display device CPU frequency
    Serial.printf(">>> Firmware Version: %s \n", FirmwareVer); // Display current firmware version
    Serial.printf(">>> ESP ID: %X \r\n", CHIPID); // Display ESP chip ID
    Serial.printf(">>> Boot Mode: %d \n", ESP.getBootMode()); // Display boot mode
    Serial.printf(">>> Free Sketch Space: %d \n", ESP.getFreeSketchSpace()); // Display free sketch space

    WiFiClientSecure client;
    client.setTrustAnchors(&cert);
    if (!client.connect(host, httpsPort)) {
        Serial.println(">>> raw.githubusercontent.com - Connection failed"); // Display connection failure message
        Serial.println(">>> Server overloaded or unavailable..."); // Display server unavailability message
        Serial.println(">>> Or your device is not permitted to update on the system..."); // Display device update permission message
        Serial.println(">>> Please check for updates at another time..."); // Prompt to check for updates later
        Serial.printf(">>> Current version is %s \n", FirmwareVer); // Display current firmware version
        return;
    }

    client.print(String("GET ") + URL_fw_Version + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "User-Agent: BuildFailureDetectorESP8266\r\n" +
                 "Connection: close\r\n\r\n");

    // Check content from the Version file
    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            Serial.println(">>> Headers received"); // Indicate that headers have been received
            break;
        }
    }

    String payload = client.readString(); // Get the request response payload
    Serial.println(payload); // Display payload received
    DynamicJsonDocument jsonBuffer(1024);

    auto error = deserializeJson(jsonBuffer, payload);
    if (error) {
        Serial.print(F("deserializeJson() failed with code ")); // Display JSON deserialization failure message
        Serial.println(error.c_str());
        return;
    }

    String author_prod = jsonBuffer["author"];
    String version_prod = jsonBuffer["main"]["version"];

    // serializeJson(jsonBuffer, Serial);

    if (version_prod.equals(FirmwareVer)) {
        Serial.println(">>> Device already on latest firmware version"); // Display message indicating already on latest firmware version
        
        Serial.println("The current version "); // Display message about current version
        
        Serial.println("      is the latest."); // Display message indicating latest version
        
        Serial.println("> > > > > > > > > > "); // Indicate end of update check
        delay(1500);
    } else {
        Serial.print(">>> New firmware detected: "); // Display message indicating new firmware detected
        Serial.println(version_prod);
        
        Serial.println("New firmware detected"); // Display message indicating new firmware detected
        
        Serial.println(FirmwareVer);
        Serial.println(" -> ");
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

        t_httpUpdate_return ret = ESPhttpUpdate.update(client, URL_fw_Bin);

        switch (ret) {
            case HTTP_UPDATE_FAILED:
                Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str()); // Display HTTP update failed message
                Serial.println(">>> Server overloaded or unavailable..."); // Display server unavailability message
                Serial.println(">>> Or your device is not permitted to update on the system..."); // Display device update permission message
                Serial.println(">>> Please check for updates at another time..."); // Prompt to check for updates later
                Serial.printf(">>> Current version is %s \n", FirmwareVer); // Display current firmware version
                
                Serial.println("> Skip updated...ERR"); // Display skipping update message
                delay(2000);
                break;

            case HTTP_UPDATE_NO_UPDATES:
                Serial.println("HTTP_UPDATE_NO_UPDATES"); // Display message indicating no updates available
                Serial.println(">>> The current version is the latest."); // Display message indicating already on latest version
                
                Serial.println("The current version "); // Display message about current version
                
                Serial.println("      is the latest."); // Display message indicating latest version
                
                Serial.println("> > > > > > > > > > "); // Indicate end of update check
                delay(1500);
                break;

            case HTTP_UPDATE_OK:
                Serial.println("HTTP_UPDATE_OK"); // Display message indicating update successful
                break;
        }
    }
    
    Serial.println("\n<<<<<<<<<< Done Check FOTA \n"); // Indicate completion of FOTA update process
}


void smartConfig_ndb() {
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
  while (millis() - startTime < timeoutSeconds * 1000) {
    // Delay for 1 second
    delay(1000);

    // Check if timeout is reached (100 seconds)
    Serial.printf("Time remaining for connection: %d seconds\n", timeoutSeconds - (millis() - startTime) / 1000);
    
    // Check if SmartConfig connection is successful
    if (WiFi.smartConfigDone()) {
      // Retrieve the configured SSID (network name)
      String qsid = WiFi.SSID();

      // Retrieve the configured password
      String qpass = WiFi.psk();

      // Print message to serial monitor indicating SmartConfig success
      Serial.println("SmartConfig Success");

      // Print message to serial monitor indicating start of information
      Serial.println("Begin SmartConfig!!!");

      // Print retrieved SSID to serial monitor
      Serial.printf("SSID: ");
      Serial.println(qsid);

      // Print retrieved password to serial monitor
      Serial.printf("Password: ");
      Serial.println(qpass);

      // (Optional) Activate a signal (bell or return) upon successful connection (commented out)
      // digitalWrite(PIN_signal_Bell, ESP_NB_ON);

      // Check if SSID and password have valid lengths
      if (qsid.length() > SIZE_NAME_SSID && qpass.length() >= SIZE_CHAR_PASS) {
        // Print message to serial monitor indicating EEPROM clearing
        Serial.println("Clearing EEPROM");

        // Clear the EEPROM memory (used for non-volatile storage)
        for (int i = START_EEPROM_SSID; i < END_EEPROM_PASS; ++i) {
          EEPROM.write(i, 0);
        }

        // Print message to serial monitor indicating start of SSID writing to EEPROM
        Serial.println("Writing EEPROM SSID:");

        // Write the SSID to EEPROM (one character at a time)
        for (int i = 0; i < (int)qsid.length(); ++i) {
          EEPROM.write(i + START_EEPROM_SSID, qsid[i]);
          Serial.printf("Wrote [%d]: ", i + START_EEPROM_SSID);
          Serial.println(qsid[i]);
        }

        // Print message to serial monitor indicating start of password writing to EEPROM
        Serial.println("Writing EEPROM password:");

        // Write the password to EEPROM (one character at a time, starting at a different address)
        for (int i = 0; i < (int)qpass.length(); ++i) {
          EEPROM.write(i + START_EEPROM_PASS, qpass[i]);
          Serial.printf("Wrote [%d]: ", i + START_EEPROM_PASS);
          Serial.println(qpass[i]);
        }

        // Commit changes to EEPROM memory
        EEPROM.commit();

        // (Optional) Deactivate the signal upon successful saving (commented out)
        // digitalWrite(PIN_signal_Bell, ESP_NB_OFF);

        // Print message to serial monitor indicating successful saving of ID and password
        Serial.println("EEPROM.commit() =>>> Saved ID & Pass Wifi");

        // Delay for 200 milliseconds
        delay(200);

        // Exit the loop
        break;
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

bool bool_Test_Wifi(void)
{
	int c = 0;
	Serial.println("");
	Serial.println("Waiting for Wifi to connect");
	Serial.println("=========  Note =========");
	Serial.println("WL_NO_SHIELD        = 255");
	Serial.println("WL_IDLE_STATUS      = 0");
	Serial.println("WL_NO_SSID_AVAIL    = 1");
	Serial.println("WL_SCAN_COMPLETED   = 2");
	Serial.println("WL_CONNECTED        = 3");
	Serial.println("WL_CONNECT_FAILED   = 4");
	Serial.println("WL_CONNECTION_LOST  = 5");
	Serial.println("WL_WRONG_PASSWORD   = 6");
	Serial.println("WL_DISCONNECTED     = 7");
	Serial.println("========================");
	while (c < 40)
	{
		Serial.println("");
		if (WiFi.status() == WL_CONNECTED)
		{
			/* Chuông báo ok */
			digitalWrite(PIN_signal_Bell, 0);
			delay(1500);
			digitalWrite(PIN_signal_Bell, 1);
			return true;
		}
		/* Chuông báo ok */
		digitalWrite(PIN_signal_Bell, 0);
		delay(300);
		digitalWrite(PIN_signal_Bell, 1);
		delay(300);
		Serial.print(WiFi.status());
		Serial.print(" -> ");
		c++;
	}
	Serial.println("");
	Serial.println("Connect timed out, opening AP");
	return false;
}

