#include "BluetoothA2DPSource.h"
#include <Adafruit_NeoPixel.h>

// Пины и параметры
#define LED_PIN    27
#define NUM_LEDS   30

// Переменные
BluetoothA2DPSource a2dp_source;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

int sampleIndex = 0;
long sum = 0;

// Режимы подсветки
enum LightMode {
  MODE_COLOR_MUSIC,
  MODE_RAINBOW,
  MODE_STATIC_COLOR
};
LightMode currentMode = MODE_COLOR_MUSIC;

uint32_t staticColor = strip.Color(255, 0, 0); // Красный по умолчанию

// Callback: получаем данные с Bluetooth
void audio_data_callback(const uint8_t *data, uint32_t length)
{
    a2dp_source.feed(data, length); // Отправляем звук дальше (на динамик)

    if (currentMode == MODE_COLOR_MUSIC) {
        for (int i = 0; i < length / 2; i++) {
            int16_t sample = ((int16_t*)data)[i];
            sum += abs(sample);
            sampleIndex++;
            if (sampleIndex >= 1024) {
                int avg = sum / 1024;
                showAudioLevel(avg);
                sum = 0;
                sampleIndex = 0;
            }
        }
    }
}

// Инициализация
void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("Запуск...");

    strip.begin();
    strip.show();

    a2dp_source.start("MySmartSpeaker", audio_data_callback);
}

// Основной цикл
void loop()
{
    // Можно добавлять переключение режимов здесь
    delay(100);
}

// === Функции отображения световых эффектов ===

// Цветомузыкальный уровень
void showAudioLevel(int level)
{
    int ledsOn = map(level, 0, 32767, 0, NUM_LEDS);
    for (int i = 0; i < NUM_LEDS; i++) {
        if (i < ledsOn) {
            strip.setPixelColor(i, getColorForLevel(i, ledsOn));
        } else {
            strip.setPixelColor(i, 0, 0, 0);
        }
    }
    strip.show();
}

// Генерация цвета по позиции
uint32_t getColorForLevel(int pos, int maxLevel)
{
    uint8_t r, g, b;
    uint8_t hue = map(pos, 0, maxLevel, 0, 255);
    hsvToRgb(hue, 255, 255, &r, &g, &b);
    return strip.Color(r, g, b);
}

// Конвертация HSV -> RGB
void hsvToRgb(uint8_t h, uint8_t s, uint8_t v, uint8_t* r, uint8_t* g, uint8_t* b)
{
    if (s == 0) {
        *r = *g = *b = v;
        return;
    }

    h = h * 6;
    uint8_t region = h / 43;
    uint8_t remainder = (h - (region * 43)) * 6;

    uint8_t p = (v * (255 - s)) >> 8;
    uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
        case 0:  *r = v; *g = t; *b = p; break;
        case 1:  *r = q; *g = v; *b = p; break;
        case 2:  *r = p; *g = v; *b = t; break;
        case 3:  *r = p; *g = q; *b = v; break;
        case 4:  *r = t; *g = p; *b = v; break;
        default: *r = v; *g = p; *b = q; break;
    }
}

// === Дополнительные режимы (можно вызывать из других функций или по кнопке/команде) ===

// Статичный цвет
void setStaticColor(uint32_t color)
{
    currentMode = MODE_STATIC_COLOR;
    staticColor = color;
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, staticColor);
    }
    strip.show();
}

// Радуга
void rainbowCycle(int wait) {
    currentMode = MODE_RAINBOW;
    uint16_t i, j;
    for(j=0; j<256*5; j++) { // 5 циклов радуги
        for(i=0; i< strip.numPixels(); i++) {
            strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
        }
        strip.show();
        delay(wait);
    }
}

// Вспомогательная функция для радуги
uint32_t Wheel(byte WheelPos) {
    if(WheelPos < 85) {
        return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    } else if(WheelPos < 170) {
        WheelPos -= 85;
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else {
        WheelPos -= 170;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
}
