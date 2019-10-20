// M5StickC Demo
// THIS IS JUST A DEMO OF FEATURES
// INCLUDES CLOCK WHICH CAN BE SET VIA GIT REQUEST TO worldtimeapi.org
// Battery Voltage
// PLOTS SENSORS - Accelerometer, Gyroscope, Hall.
// by brianmoreau.com

#include <M5StickC.h>
#include <WiFi.h>
WiFiClient client;

const char* ssid     = "MYWIFINAME";
const char* password = "MYWIFIPASSWORD";

const char* hostTime = "worldtimeapi.org";
String urlTime = "/api/timezone/Europe/London.txt";
int hourT,minT,secT;
int btnA = 39; // SELECT BUTTON 
int btnB = 37; // MENU BUTTON 
unsigned long buttonBounce;
int dispMode = 0;
int numberOfModes = 5;
bool selected = false;
unsigned long timeDuration = 1000;
unsigned long lastTime;
RTC_TimeTypeDef RTC_TimeStruct;
bool wifiOK = false;
unsigned long startTime;

int xPlot = 0;
int lastGX = 0;
int lastGY = 0;
int lastGZ = 0;
int lastAX = 0;
int lastAY = 0;
int lastAZ = 0;

void setup() {
  
  M5.begin();
  pinMode(btnA,INPUT_PULLUP);
  pinMode(btnB,INPUT_PULLUP);
  M5.Lcd.setRotation(1);
  homeScreen();
  
}

void loop() {

  // button debounce
  // only read button if x time passed since last press
  if (millis() - buttonBounce > 300) {
    if (digitalRead(btnB) == LOW) {
      if (dispMode < numberOfModes) {
        dispMode++;
        showMode();
      } else {
        dispMode = 0;
        homeScreen();
      }
      // log last time pressed
      buttonBounce = millis();
      // mode not selected
      selected = false;
    }
    if (digitalRead(btnA) == LOW) {
      selected = true;
      // log last time pressed
      buttonBounce = millis();
    }
  }

  if (selected == true) {
    // TIME
    if (dispMode == 0) {
    } else if (dispMode == 1) {
      // SHOW TIME EVERY SECOND
      if (millis() - lastTime > timeDuration) {
        M5.Lcd.fillScreen(GREEN);
        M5.Lcd.setCursor(10,30);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setTextColor(BLACK);
        M5.Rtc.GetTime(&RTC_TimeStruct);
        M5.Lcd.printf("%02d:",RTC_TimeStruct.Hours);
        M5.Lcd.printf("%02d:",RTC_TimeStruct.Minutes);
        M5.Lcd.printf("%02d",RTC_TimeStruct.Seconds);
        lastTime = millis();
      }
    } else if (dispMode == 2) {
      // UPDATE TIME VIA WEB 
      conWifi();
      if (wifiOK) {
        getTime();
      }
      // do not repeat
      selected = false;
      //////////////////////////////////
    } else if (dispMode == 3) {
      // SHOW BATTERY LEVEL
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(10,10);
      M5.Lcd.printf("%.3fV",M5.Axp.GetVbatData() * 1.1 / 1000);
      // do not repeat
      selected = false;
      ////////////////////////////////////
    } else if (dispMode == 4) {
      // Gyroscope
      float gyroX_f, gyroY_f, gyroZ_f;
      M5.MPU6886.Init();
      M5.MPU6886.getGyroData(&gyroX_f, &gyroY_f,&gyroZ_f); // o/s
      // PLOT SCREEN IS 160x80
      int gX = map(gyroX_f, -300,300, 0, 80);
      int gY = map(gyroY_f, -300,300, 0, 80);
      int gZ = map(gyroZ_f, -300,300, 0, 80);
      M5.Lcd.drawLine(xPlot, lastGX, xPlot+1, gX, TFT_BLUE);
      M5.Lcd.drawLine(xPlot, lastGY, xPlot+1, gY, TFT_GREEN);
      M5.Lcd.drawLine(xPlot, lastGZ, xPlot+1, gZ, TFT_YELLOW);
      lastGX = gX;
      lastGY = gY;
      lastGZ = gZ;
      if (xPlot < 160) {
        xPlot++;
      } else {
        xPlot = 0;
        M5.Lcd.fillScreen(BLACK);
      }
    } else if (dispMode == 5) {
      // Accelerometer
      float accX_f, accY_f, accZ_f;
      M5.MPU6886.Init();
      M5.MPU6886.getAccelData(&accX_f,&accY_f,&accZ_f); // mg
      // PLOT SCREEN IS 160x80
      int aX = map(accX_f, -10,10, 0, 80);
      int aY = map(accY_f, -10,10, 0, 80);
      int aZ = map(accZ_f, -10,10, 0, 80);
      M5.Lcd.drawLine(xPlot, lastAX, xPlot+1, aX, TFT_BLUE);
      M5.Lcd.drawLine(xPlot, lastAY, xPlot+1, aY, TFT_GREEN);
      M5.Lcd.drawLine(xPlot, lastAZ, xPlot+1, aZ, TFT_YELLOW);
      lastAX = aX;
      lastAY = aY;
      lastAZ = aZ;
      if (xPlot < 160) {
        xPlot++;
      } else {
        xPlot = 0;
        M5.Lcd.fillScreen(BLACK);
      }
    } else if (dispMode == 6) {
      // Hall
      int hes = hallRead();
      int aX = map(hes, -100,100, 0, 80);
      M5.Lcd.drawLine(xPlot, lastAX, xPlot+1, aX, TFT_YELLOW);
      lastAX = aX;
      if (xPlot < 160) {
        xPlot++;
      } else {
        xPlot = 0;
        M5.Lcd.fillScreen(BLACK);
      }
      // cos this is fast
      delay(50);
    } else if (dispMode == 7) {
      
    } else if (dispMode == 8) {
      
    }
  }

} // end loop

void showMode() {
  M5.Lcd.setTextSize(2);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(0,10);
  if (dispMode == 0) {
    
  } else if (dispMode == 1) {
    M5.Lcd.print("CLOCK");
  } else if (dispMode == 2) {
    M5.Lcd.print("UPDATE TIME");
  } else if (dispMode == 3) {
    M5.Lcd.print("BATTERY");
  } else if (dispMode == 4) {
    M5.Lcd.print("GYROSCOPE");
  } else if (dispMode == 5) {
    M5.Lcd.print("Accelerometer");
  } else if (dispMode == 6) {
    M5.Lcd.print("Hall Sensor");
  } else if (dispMode == 7) {
    M5.Lcd.print("MODE 7");
  } else if (dispMode == 8) {
    M5.Lcd.print("MODE 8");
  }
}

void homeScreen() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(30,20);
  M5.Lcd.printf("^^^ Select");
  M5.Lcd.setCursor(50,50);
  M5.Lcd.printf("Menu >>");
}

void conWifi() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0,0);
  M5.Lcd.printf("Connecting");
  startTime = millis();
  bool timeOut = false;
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - startTime > 10000) {
      timeOut = true;
      break;
    }
  }

  if (timeOut) {
    Serial.println("WiFi Fail");
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0,0);
    M5.Lcd.printf("WiFi Fail");
  } else {
    Serial.println("WiFi connected");
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0,0);
    M5.Lcd.printf("WiFi OK");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    wifiOK = true;
  }
}

void getTime() {
  if (!client.connect(hostTime, 80)) {
    Serial.println("connection failed");
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0,0);
    M5.Lcd.printf("Host Fail");
    return;
  } else {
    Serial.println("Connected to host");
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0,0);
    M5.Lcd.printf("Host OK");
    client.print(String("GET ") + urlTime + " HTTP/1.1\r\n" +
      "Host: " + hostTime + "\r\n" +
      "Connection: close\r\n\r\n");
    startTime = millis();
    while (client.available() == 0) {
      if (millis() - startTime > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }
    int lineNum = 0;
    bool dataValid = false;
    String timeDateData;
    while(client.available()) {
      String line = client.readStringUntil('\r\n');
      // first line
      // HTTP/1.1 404 Not Found
      // HTTP/1.1 200 OK
      line.trim();
      if (lineNum == 0 && line == "HTTP/1.1 200 OK") {
        dataValid = true;
      }
      if (lineNum == 14 && dataValid == true) {
        timeDateData = line;
      }
      lineNum++;
    }
    if (dataValid) {
      hourT = timeDateData.substring(21,23).toInt();
      minT = timeDateData.substring(24,26).toInt();
      secT = timeDateData.substring(27,29).toInt();
      RTC_TimeTypeDef TimeStruct;
      TimeStruct.Hours   = hourT;
      TimeStruct.Minutes = minT;
      TimeStruct.Seconds = secT;
      M5.Rtc.SetTime(&TimeStruct);
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0,0);
      M5.Lcd.printf("Time OK");
    } else {
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0,0);
      M5.Lcd.printf("Time Fail");
    }  
  }
}
