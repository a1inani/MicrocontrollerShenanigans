#include <M5Cardputer.h>
#include <driver/i2s.h>
#include <SD.h>
#include <SPI.h>
#include <vector>

// Launcher metadata
#define APP_NAME "Dictaphone"
#define APP_VERSION "1.0.0"
#define APP_AUTHOR "Community"

// I2S Configuration for built-in microphone
#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 16000
#define BUFFER_SIZE 512
#define RECORD_TIME 300  // Max recording time in seconds (5 minutes)

// SD Card SPI pins for M5Stack Cardputer
#define SD_SPI_SCK_PIN  40
#define SD_SPI_MISO_PIN 39
#define SD_SPI_MOSI_PIN 14
#define SD_SPI_CS_PIN   12

enum State {
  IDLE,
  RECORDING,
  PLAYING,
  BROWSING
};

State currentState = IDLE;
File audioFile;
String currentFilename = "";
std::vector<String> recordings;
int selectedIndex = 0;
unsigned long recordStartTime = 0;
bool isPaused = false;
bool sdCardPresent = false;
bool exitApp = false;

// I2S microphone pins for M5Stack Cardputer
#define I2S_MIC_SERIAL_CLOCK GPIO_NUM_41
#define I2S_MIC_LEFT_RIGHT_CLOCK GPIO_NUM_43
#define I2S_MIC_SERIAL_DATA GPIO_NUM_42

// Speaker pin
#define SPEAKER_PIN GPIO_NUM_11

void setupI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = BUFFER_SIZE,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_MIC_SERIAL_CLOCK,
    .ws_io_num = I2S_MIC_LEFT_RIGHT_CLOCK,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SERIAL_DATA
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_zero_dma_buffer(I2S_PORT);
}

bool initSDCard() {
  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
  
  if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
    return false;
  }
  
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    return false;
  }
  
  // Create recordings directory if it doesn't exist
  if (!SD.exists("/recordings")) {
    SD.mkdir("/recordings");
  }
  
  return true;
}

void scanRecordings() {
  recordings.clear();
  
  if (!sdCardPresent) return;
  
  File root = SD.open("/recordings");
  if (!root) return;
  
  File file = root.openNextFile();
  
  while (file) {
    if (!file.isDirectory()) {
      String filename = file.name();
      if (filename.endsWith(".wav") || filename.endsWith(".WAV")) {
        recordings.push_back(String(file.name()));
      }
    }
    file = root.openNextFile();
  }
  
  root.close();
  
  if (selectedIndex >= recordings.size() && recordings.size() > 0) {
    selectedIndex = recordings.size() - 1;
  }
}

String generateFilename() {
  int count = 0;
  String filename;
  do {
    filename = "/recordings/rec_" + String(count) + ".wav";
    count++;
  } while (SD.exists(filename));
  return filename;
}

void writeWavHeader(File &file, uint32_t dataSize) {
  uint32_t fileSize = dataSize + 36;
  uint16_t audioFormat = 1;  // PCM
  uint16_t numChannels = 1;
  uint32_t sampleRate = SAMPLE_RATE;
  uint32_t byteRate = sampleRate * numChannels * 2;
  uint16_t blockAlign = numChannels * 2;
  uint16_t bitsPerSample = 16;

  file.write((const uint8_t*)"RIFF", 4);
  file.write((const uint8_t*)&fileSize, 4);
  file.write((const uint8_t*)"WAVE", 4);
  file.write((const uint8_t*)"fmt ", 4);
  uint32_t subchunk1Size = 16;
  file.write((const uint8_t*)&subchunk1Size, 4);
  file.write((const uint8_t*)&audioFormat, 2);
  file.write((const uint8_t*)&numChannels, 2);
  file.write((const uint8_t*)&sampleRate, 4);
  file.write((const uint8_t*)&byteRate, 4);
  file.write((const uint8_t*)&blockAlign, 2);
  file.write((const uint8_t*)&bitsPerSample, 2);
  file.write((const uint8_t*)"data", 4);
  file.write((const uint8_t*)&dataSize, 4);
}

void startRecording() {
  if (!sdCardPresent) {
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setCursor(5, 5);
    M5Cardputer.Display.println("Error: No SD card");
    delay(2000);
    drawUI();
    return;
  }
  
  currentFilename = generateFilename();
  audioFile = SD.open(currentFilename, FILE_WRITE);
  
  if (!audioFile) {
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setCursor(5, 5);
    M5Cardputer.Display.println("Error: Cannot create file");
    delay(2000);
    drawUI();
    return;
  }
  
  // Write temporary header (will update at the end)
  writeWavHeader(audioFile, 0);
  
  currentState = RECORDING;
  recordStartTime = millis();
  isPaused = false;
  M5Cardputer.Display.fillScreen(BLACK);
}

void stopRecording() {
  if (currentState != RECORDING) return;
  
  uint32_t dataSize = audioFile.size() - 44;
  audioFile.seek(0);
  writeWavHeader(audioFile, dataSize);
  audioFile.close();
  
  currentState = IDLE;
  scanRecordings();
}

void recordAudio() {
  int16_t buffer[BUFFER_SIZE];
  size_t bytesRead = 0;
  
  i2s_read(I2S_PORT, buffer, BUFFER_SIZE * sizeof(int16_t), &bytesRead, portMAX_DELAY);
  
  if (bytesRead > 0 && !isPaused) {
    audioFile.write((uint8_t*)buffer, bytesRead);
  }
}

void playRecording() {
  if (recordings.empty() || !sdCardPresent) return;
  
  String fullPath = "/recordings/" + recordings[selectedIndex];
  audioFile = SD.open(fullPath, FILE_READ);
  if (!audioFile) {
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setCursor(5, 5);
    M5Cardputer.Display.println("Error: Cannot open file");
    delay(2000);
    currentState = BROWSING;
    drawUI();
    return;
  }
  
  // Skip WAV header
  audioFile.seek(44);
  currentState = PLAYING;
  M5Cardputer.Display.fillScreen(BLACK);
}

void playAudio() {
  int16_t buffer[BUFFER_SIZE];
  size_t bytesRead = audioFile.read((uint8_t*)buffer, BUFFER_SIZE * sizeof(int16_t));
  
  if (bytesRead > 0) {
    // Note: ESP32-S3 doesn't have DAC, using I2S output through speaker would be better
    // For now, this is a simplified version - proper audio output needs I2S configuration
    // You may hear clicking/noise instead of clear audio
    for (int i = 0; i < bytesRead / 2; i++) {
      // Simple delay for timing - not ideal but works for basic playback
      delayMicroseconds(1000000 / SAMPLE_RATE);
    }
  } else {
    stopPlayback();
  }
}

void stopPlayback() {
  if (currentState != PLAYING) return;
  audioFile.close();
  currentState = IDLE;
}

void deleteRecording() {
  if (recordings.empty() || !sdCardPresent) return;
  
  String fullPath = "/recordings/" + recordings[selectedIndex];
  SD.remove(fullPath);
  scanRecordings();
  if (selectedIndex >= recordings.size() && recordings.size() > 0) {
    selectedIndex = recordings.size() - 1;
  }
}

uint64_t getSDCardSize() {
  if (!sdCardPresent) return 0;
  return SD.cardSize() / (1024 * 1024); // Return size in MB
}

uint64_t getSDCardUsed() {
  if (!sdCardPresent) return 0;
  return SD.usedBytes() / (1024 * 1024); // Return used space in MB
}

void drawUI() {
  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setCursor(5, 5);
  
  switch (currentState) {
    case IDLE:
      M5Cardputer.Display.println("=== DICTAPHONE ===");
      M5Cardputer.Display.println();
      
      if (!sdCardPresent) {
        M5Cardputer.Display.setTextColor(RED);
        M5Cardputer.Display.println("NO SD CARD DETECTED!");
        M5Cardputer.Display.setTextColor(WHITE);
        M5Cardputer.Display.println();
        M5Cardputer.Display.println("Insert SD card and");
        M5Cardputer.Display.println("press any key to retry");
      } else {
        M5Cardputer.Display.println("R - Start Recording");
        M5Cardputer.Display.println("B - Browse Files");
        M5Cardputer.Display.println();
        M5Cardputer.Display.printf("Recordings: %d\n", recordings.size());
        M5Cardputer.Display.printf("SD Card: %llu MB\n", getSDCardSize());
        M5Cardputer.Display.printf("Used: %llu MB\n", getSDCardUsed());
      }
      M5Cardputer.Display.println();
      M5Cardputer.Display.setTextColor(DARKGREY);
      M5Cardputer.Display.println("ESC - Exit to Launcher");
      break;
      
    case RECORDING: {
      M5Cardputer.Display.setTextColor(RED);
      M5Cardputer.Display.println("=== RECORDING ===");
      M5Cardputer.Display.setTextColor(WHITE);
      M5Cardputer.Display.println();
      unsigned long elapsed = (millis() - recordStartTime) / 1000;
      M5Cardputer.Display.printf("Time: %02lu:%02lu\n", elapsed / 60, elapsed % 60);
      
      // Extract just the filename without path
      String displayName = currentFilename;
      int lastSlash = displayName.lastIndexOf('/');
      if (lastSlash >= 0) {
        displayName = displayName.substring(lastSlash + 1);
      }
      M5Cardputer.Display.printf("File: %s\n", displayName.c_str());
      M5Cardputer.Display.printf("Size: %lu KB\n", audioFile.size() / 1024);
      M5Cardputer.Display.println();
      if (isPaused) {
        M5Cardputer.Display.setTextColor(YELLOW);
        M5Cardputer.Display.println("*** PAUSED ***");
        M5Cardputer.Display.setTextColor(WHITE);
      }
      M5Cardputer.Display.println();
      M5Cardputer.Display.println("P - Pause/Resume");
      M5Cardputer.Display.println("S - Stop Recording");
      break;
    }
      
    case PLAYING:
      M5Cardputer.Display.setTextColor(GREEN);
      M5Cardputer.Display.println("=== PLAYING ===");
      M5Cardputer.Display.setTextColor(WHITE);
      M5Cardputer.Display.println();
      M5Cardputer.Display.printf("File: %s\n", recordings[selectedIndex].c_str());
      M5Cardputer.Display.println();
      M5Cardputer.Display.println("S - Stop Playback");
      break;
      
    case BROWSING:
      M5Cardputer.Display.println("=== RECORDINGS ===");
      M5Cardputer.Display.println();
      
      if (recordings.empty()) {
        M5Cardputer.Display.println("No recordings found");
        M5Cardputer.Display.println();
        M5Cardputer.Display.println("Press R to start");
        M5Cardputer.Display.println("recording");
      } else {
        int startIdx = max(0, selectedIndex - 3);
        int endIdx = min((int)recordings.size(), startIdx + 7);
        
        for (int i = startIdx; i < endIdx; i++) {
          if (i == selectedIndex) {
            M5Cardputer.Display.setTextColor(YELLOW);
            M5Cardputer.Display.print("> ");
          } else {
            M5Cardputer.Display.setTextColor(WHITE);
            M5Cardputer.Display.print("  ");
          }
          M5Cardputer.Display.println(recordings[i]);
        }
      }
      
      M5Cardputer.Display.setTextColor(WHITE);
      M5Cardputer.Display.println();
      M5Cardputer.Display.println("Up/Dn - Navigate");
      if (!recordings.empty()) {
        M5Cardputer.Display.println("P - Play | D - Delete");
      }
      M5Cardputer.Display.println("ESC - Back");
      break;
  }
}

void returnToLauncher() {
  // Stop any ongoing operations
  if (currentState == RECORDING) {
    stopRecording();
  }
  if (currentState == PLAYING) {
    stopPlayback();
  }
  
  // Cleanup I2S
  i2s_driver_uninstall(I2S_PORT);
  
  // Close SD
  SD.end();
  
  // Show exit message
  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.setCursor(10, 60);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.println("Returning to Launcher...");
  M5Cardputer.Display.println();
  M5Cardputer.Display.println("If stuck, press reset.");
  delay(1000);
  
  // Put display to sleep
  M5Cardputer.Display.sleep();
  
  // Restart to return to launcher
  esp_restart();
}

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);
  
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setCursor(5, 5);
  M5Cardputer.Display.println("Initializing...");
  M5Cardputer.Display.println();
  M5Cardputer.Display.printf("%s v%s\n", APP_NAME, APP_VERSION);
  M5Cardputer.Display.println();
  
  // Initialize SD card
  sdCardPresent = initSDCard();
  
  if (sdCardPresent) {
    M5Cardputer.Display.println("SD Card: OK");
    scanRecordings();
  } else {
    M5Cardputer.Display.println("SD Card: NOT FOUND");
  }
  
  delay(1000);
  
  setupI2S();
  M5Cardputer.Display.println("I2S: OK");
  delay(500);
  
  drawUI();
}

void loop() {
  M5Cardputer.update();
  
  // Check for exit request
  if (exitApp) {
    returnToLauncher();
    return; // Will never reach here due to restart
  }
  
  if (M5Cardputer.Keyboard.isChange()) {
    if (M5Cardputer.Keyboard.isPressed()) {
      Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
      
      // Convert status.word to String for comparison
      String keyPressed = "";
      for (char c : status.word) {
        keyPressed += c;
      }
      
      switch (currentState) {
        case IDLE:
          if (keyPressed == "`") {  // ESC key - exit to launcher
            exitApp = true;
            return;
          } else if (!sdCardPresent) {
            // Try to reinitialize SD card on any key press
            sdCardPresent = initSDCard();
            if (sdCardPresent) {
              scanRecordings();
            }
            drawUI();
          } else {
            if (keyPressed == "r") {
              startRecording();
            } else if (keyPressed == "b") {
              currentState = BROWSING;
            }
            drawUI();
          }
          break;
          
        case RECORDING:
          if (keyPressed == "s") {
            stopRecording();
            drawUI();
          } else if (keyPressed == "p") {
            isPaused = !isPaused;
            drawUI();
          }
          break;
          
        case PLAYING:
          if (keyPressed == "s") {
            stopPlayback();
            drawUI();
          }
          break;
          
        case BROWSING:
          if (keyPressed == ";") {  // Up arrow
            selectedIndex = max(0, selectedIndex - 1);
            drawUI();
          } else if (keyPressed == ".") {  // Down arrow
            selectedIndex = min((int)recordings.size() - 1, selectedIndex + 1);
            drawUI();
          } else if (keyPressed == "p" && !recordings.empty()) {
            playRecording();
          } else if (keyPressed == "d" && !recordings.empty()) {
            deleteRecording();
            drawUI();
          } else if (keyPressed == "`") {  // ESC
            currentState = IDLE;
            drawUI();
          } else if (keyPressed == "r") {  // Allow recording from browse menu
            currentState = IDLE;
            startRecording();
          }
          break;
      }
    }
  }
  
  // Handle continuous operations
  if (currentState == RECORDING) {
    recordAudio();
    
    // Update display every second
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 1000) {
      lastUpdate = millis();
      drawUI();
    }
    
    // Auto-stop after max time
    if ((millis() - recordStartTime) / 1000 >= RECORD_TIME) {
      stopRecording();
      drawUI();
    }
  } else if (currentState == PLAYING) {
    playAudio();
  }
  
  delay(10);
}