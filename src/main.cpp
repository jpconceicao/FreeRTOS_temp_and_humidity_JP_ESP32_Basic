/* --------------------------------------------------------------------------------------
    Author: Jorge Palma Conceição
    Date:   20-07-2024
    Resume: Code made to test FreeRTOS in development board (JP ESP32 Basic).
            The application read temperature and humidity and displays in the OLED display,
            and if the user press the button he can change from one to another.
  -------------------------------------------------------------------------------------- */

#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <DHT_U.h>

/* Defining pins and DHT type, setting DHT */
#define     DHTPIN      13
#define     DHTTYPE     DHT11
#define     BUTTON_PIN  34  
DHT dht(DHTPIN, DHTTYPE);

/* Setting display OLED */
#define     SCREEN_WIDTH 128
#define     SCREEN_HEIGHT 64
#define     OLED_RESET    -1
#define     I2C_SDA       21
#define     I2C_SCL       22
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/* Creating Variables to store values and manipule display */
float temperature;
float humidity;
bool showTemperature;

SemaphoreHandle_t semaphore = NULL;

void readDHT(void * parameters)
{
    for(;;)
    {
        humidity = dht.readHumidity();
        temperature = dht.readTemperature();
        if (isnan(humidity) || isnan(temperature))
        {
            Serial.println("Error in readings of humidity and temperature");
        }
        else
        {
            Serial.print(F("Humidity: "));
            Serial.print(humidity);
            Serial.print(F("%  Temperature: "));
            Serial.print(temperature);
            Serial.println(F("ºC"));
        }
        xSemaphoreGive(semaphore);  // Release semaphore to update the display!
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void updateDisplay(void * parameters)
{
    for (;;) 
    {
    if (xSemaphoreTake(semaphore, (TickType_t) 10) == pdTRUE) 
    {
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 10);
      if (showTemperature) {
        display.println(F("Temp: "));
        display.println(F(""));
        display.print(temperature);
        display.print(F(" C"));
      } else {
        display.println(F("Umid: "));
         display.println(F(""));
        display.print(humidity);
        display.print(F(" %"));
      }
      display.display();
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void buttonHandler(void * parameters)
{
    for(;;)
    {
        if(digitalRead(BUTTON_PIN) == LOW)
        {
            showTemperature = !showTemperature;
            xSemaphoreGive(semaphore);
            Serial.println(F("Button Pressed"));
            vTaskDelay(pdMS_TO_TICKS(500));  // Delay to debounce
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void setup() 
{
    Serial.begin(115200);
    delay(100);

    Wire.begin(I2C_SDA, I2C_SCL);

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("Failed to init OLED display"));
        for(;;);
    }
    display.display();
    delay(2000);
    display.clearDisplay();
    display.display();

    dht.begin();

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    semaphore = xSemaphoreCreateBinary();

    xTaskCreatePinnedToCore(readDHT, "ReadDHT", 2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(updateDisplay, "UpdateDisplay", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(buttonHandler, "ButtonHandler", 2048, NULL, 1, NULL, 1);
}

void loop() 
{

}