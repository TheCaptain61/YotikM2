#include "DisplayManager.h"
#include <TM1637Display.h>
#include "Config.h"
#include "GlobalInstances.h"

// Создаем экземпляр дисплея
TM1637Display display(Pins::TM1637_CLK, Pins::TM1637_DIO);

void DisplayManager::begin() {
    Serial.println("🔧 Initializing TM1637 Display...");
    
    // Тест 1: Проверка подключения
    Serial.print("  Testing display connection... ");
    display.setBrightness(7);
    display.clear();
    
    // Простой тест - все сегменты
    uint8_t testSegments[] = {0xff, 0xff, 0xff, 0xff};
    display.setSegments(testSegments);
    delay(500);
    
    // Тест 2: Числа
    display.showNumberDec(8888, true);
    delay(500);
    
    // Тест 3: Текст
    uint8_t textSegments[] = {
        SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // 0
        SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // 0
        SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // 0
        SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F   // 0
    };
    display.setSegments(textSegments);
    delay(500);
    
    // Очистка
    display.clear();
    
    Serial.println("✅ OK");
    Serial.println("✅ TM1637 Display initialization complete");
}

void DisplayManager::updateDisplay(const SensorData& data, const SystemSettings& settings) {
    // Смена режима каждые 3 секунды
    if (millis() - lastModeChange > 3000) {
        currentMode = (currentMode + 1) % displayModes;
        lastModeChange = millis();
        
        Serial.print("🔄 Display mode changed to: ");
        switch(currentMode) {
            case 0: Serial.println("Temperature"); break;
            case 1: Serial.println("Humidity"); break;
            case 2: Serial.println("Soil Temperature"); break;
            case 3: Serial.println("Soil Moisture"); break;
        }
    }
    
    showNextMode(data, settings);
}

void DisplayManager::showNextMode(const SensorData& data, const SystemSettings& settings) {
    switch(currentMode) {
        case 0: // Температура воздуха
            if (!isnan(data.airTemperature)) {
                showTemperature(data.airTemperature);
            } else {
                showError("Err");
            }
            break;
            
        case 1: // Влажность воздуха
            if (!isnan(data.airHumidity)) {
                showHumidity(data.airHumidity);
            } else {
                showError("Err");
            }
            break;
            
        case 2: // Температура почвы
            if (!isnan(data.soilTemperature)) {
                showTemperature(data.soilTemperature);
            } else {
                showError("Err");
            }
            break;
            
        case 3: // Влажность почвы
            if (!isnan(data.soilMoisture)) {
                showHumidity(data.soilMoisture);
            } else {
                showError("Err");
            }
            break;
    }
}

void DisplayManager::showTemperature(float temp) {
    if (isnan(temp) || temp < -50 || temp > 100) {
        showError("Err");
        return;
    }
    
    int tempInt = round(temp);
    
    if (tempInt >= 0 && tempInt < 100) {
        // Положительная температура 0-99
        display.showNumberDecEx(tempInt, 0b01100000, false, 2, 0);
        
        uint8_t segments[4];
        if (tempInt >= 10) {
            segments[0] = display.encodeDigit(tempInt / 10);
            segments[1] = display.encodeDigit(tempInt % 10);
        } else {
            segments[0] = 0;
            segments[1] = display.encodeDigit(tempInt);
        }
        segments[2] = SEG_A | SEG_B | SEG_G | SEG_F;  // °
        segments[3] = SEG_A | SEG_D | SEG_E | SEG_F;  // C
        display.setSegments(segments);
    } else if (tempInt < 0 && tempInt > -10) {
        // Отрицательная температура -1 до -9
        uint8_t segments[4];
        segments[0] = 0b01000000; // Минус
        segments[1] = display.encodeDigit(abs(tempInt));
        segments[2] = SEG_A | SEG_B | SEG_G | SEG_F;  // °
        segments[3] = SEG_A | SEG_D | SEG_E | SEG_F;  // C
        display.setSegments(segments);
    } else {
        showError("OOR"); // Out Of Range
    }
}

void DisplayManager::showHumidity(float hum) {
    if (isnan(hum) || hum < 0 || hum > 100) {
        showError("Err");
        return;
    }
    
    int humInt = round(hum);
    
    if (humInt >= 0 && humInt <= 100) {
        if (humInt == 100) {
            // 100%
            uint8_t segments[4];
            segments[0] = display.encodeDigit(1);
            segments[1] = display.encodeDigit(0);
            segments[2] = display.encodeDigit(0);
            segments[3] = SEG_A | SEG_B | SEG_E | SEG_F | SEG_G;  // P
            display.setSegments(segments);
        } else if (humInt >= 10) {
            // 10-99%
            display.showNumberDecEx(humInt, 0b01000000, false, 2, 0);
            
            uint8_t segments[4];
            segments[0] = display.encodeDigit(humInt / 10);
            segments[1] = display.encodeDigit(humInt % 10);
            segments[2] = SEG_A | SEG_B | SEG_E | SEG_F | SEG_G;  // P
            segments[3] = 0;
            display.setSegments(segments);
        } else {
            // 0-9%
            uint8_t segments[4];
            segments[0] = 0;
            segments[1] = display.encodeDigit(humInt);
            segments[2] = SEG_A | SEG_B | SEG_E | SEG_F | SEG_G;  // P
            segments[3] = 0;
            display.setSegments(segments);
        }
    } else {
        showError("OOR");
    }
}

void DisplayManager::showMessage(const String& message) {
    if (message.length() == 4) {
        uint8_t segments[4];
        for (int i = 0; i < 4; i++) {
            char c = message.charAt(i);
            segments[i] = display.encodeDigit(c);
        }
        display.setSegments(segments);
    } else {
        // Для более коротких сообщений
        display.clear();
        for (int i = 0; i < message.length() && i < 4; i++) {
            uint8_t segments[4] = {0};
            segments[i] = display.encodeDigit(message.charAt(i));
            display.setSegments(segments);
            delay(200); // Добавляем задержку между символами
        }
    }
}

void DisplayManager::showNumber(int number, bool leadingZero) {
    display.showNumberDec(number, leadingZero);
}

void DisplayManager::showLoading(uint8_t step) {
    uint8_t loading[4] = {0};
    uint8_t pos = step % 4;
    loading[pos] = 0b01000000; // Мигающая точка
    
    display.setSegments(loading);
}

void DisplayManager::setBrightness(uint8_t brightness) {
    brightness = constrain(brightness, 0, 7);
    display.setBrightness(brightness);
    Serial.println("🔆 Display brightness: " + String(brightness));
}

void DisplayManager::clear() {
    display.clear();
}

void DisplayManager::showError(const String& error) {
    Serial.println("❌ Display error: " + error);
    
    uint8_t segments[4];
    for (int i = 0; i < 4 && i < error.length(); i++) {
        char c = error.charAt(i);
        segments[i] = display.encodeDigit(c);
    }
    display.setSegments(segments);
}