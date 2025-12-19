#include <PN5180.h>
#include <PN5180ISO14443.h>
#include <PN5180ISO15693.h>

#define PN5180_NSS  16   // swapped with BUSY
#define PN5180_BUSY 5  // swapped with NSS
#define PN5180_RST  17

PN5180ISO14443 nfc(PN5180_NSS, PN5180_BUSY, PN5180_RST);

int8_t readCard(uint8_t *uidBuffer) {
    // Buffer đầy đủ 10 byte cho activateTypeA
    uint8_t response[10] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    
    // Gọi activateTypeA trực tiếp (kind = 0 = REQA)
    int8_t rawLength = nfc.activateTypeA(response, 0);
    
    //Serial.print(F("activateTypeA rawLength: "));
    //Serial.println(rawLength);
    
    // Debug: In toàn bộ buffer
    Serial.print(F("Raw buffer: "));
    for (int i = 0; i < 10; i++) {
        if (response[i] < 0x10) Serial.print("0");
        Serial.print(response[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
    
    // 1. Kiểm tra mã lỗi từ activateTypeA
    if (rawLength <= 0) {
        //Serial.print(F("activateTypeA error: "));
        switch(rawLength) {
            case 0: Serial.println(F("No card detected")); break;
            case -1: Serial.println(F("General error")); break;
            case -2: Serial.println(F("Card error")); break;
            default: Serial.println(F("Unknown error")); break;
        }
        return rawLength; // Trả về mã lỗi nguyên bản
    }
    
    // 2. Kiểm tra độ dài UID hợp lệ
    if (rawLength != 4 && rawLength != 7 && rawLength != 10) {
        //Serial.print(F("Invalid UID length: "));
        //Serial.println(rawLength);
        return -3;
    }
    
    // 3. Kiểm tra ATQA (bytes 0-1) - phải khác 0x0000 và 0xFFFF
    uint16_t atqa = (response[1] << 8) | response[0];
    Serial.print(F("ATQA: 0x"));
    if (atqa < 0x1000) Serial.print("0");
    if (atqa < 0x0100) Serial.print("0");
    if (atqa < 0x0010) Serial.print("0");
    Serial.println(atqa, HEX);
    
    if (atqa == 0x0000 || atqa == 0xFFFF) {
        Serial.println(F("Invalid ATQA (all zeros or all FFs)"));
        return -3;
    }
    
    // 4. Kiểm tra SAK (byte 2) - phải là giá trị hợp lệ
    uint8_t sak = response[2];
    //Serial.print(F("SAK: 0x"));
    if (sak < 0x10) Serial.print("0");
    //Serial.println(sak, HEX);
    
    // Danh sách SAK hợp lệ theo ISO14443-3
    bool validSAK = false;
    switch(sak) {
        case 0x00:  // Mifare Ultralight, NTAG
        case 0x08:  // Mifare Classic 1K
        case 0x09:  // Mifare Classic 4K
        case 0x10:  // Mifare Plus 2K
        case 0x11:  // Mifare Plus 4K
        case 0x18:  // Mifare Classic 2K
        case 0x20:  // JCOP31/41
        case 0x28:  // DESFire
        case 0x88:  // Mifare Classic 1K (cascade bit set)
        case 0x89:  // Mifare Classic 4K (cascade bit set)
            validSAK = true;
            break;
    }
    
    if (!validSAK) {
        Serial.println(F("Invalid SAK value"));
        return -3;
    }
    
    // 5. Xác định loại thẻ
    Serial.print(F("Card type: "));
    switch(sak) {
        case 0x00: Serial.println(F("Mifare Ultralight/NTAG")); break;
        case 0x08: 
        case 0x88: Serial.println(F("Mifare Classic 1K")); break;
        case 0x09: 
        case 0x89: Serial.println(F("Mifare Classic 4K")); break;
        case 0x18: Serial.println(F("Mifare Classic 2K")); break;
        case 0x10: Serial.println(F("Mifare Plus 2K")); break;
        case 0x11: Serial.println(F("Mifare Plus 4K")); break;
        case 0x20: Serial.println(F("JCOP31/41")); break;
        case 0x28: Serial.println(F("DESFire")); break;
        default: Serial.println(F("Unknown")); break;
    }
    
    // 6. Kiểm tra và xử lý UID theo độ dài
    if (rawLength == 4) {
        // Kiểm tra UID 4-byte
        if (!validate4ByteUID(&response[3])) {
            Serial.println(F("Invalid 4-byte UID"));
            return -3;
        }
        
        // Copy UID vào buffer đầu ra
        for (int i = 0; i < 4; i++) {
            uidBuffer[i] = response[3 + i];
        }
        
        Serial.print(F("Valid 4-byte UID: "));
        for (int i = 0; i < 4; i++) {
            if (uidBuffer[i] < 0x10) Serial.print("0");
            Serial.print(uidBuffer[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
        
        return 4;
    }
    else if (rawLength == 7) {
        // Kiểm tra UID 7-byte
        if (!validate7ByteUID(&response[3])) {
            Serial.println(F("Invalid 7-byte UID"));
            return -3;
        }
        
        // Copy UID vào buffer đầu ra
        for (int i = 0; i < 7; i++) {
            uidBuffer[i] = response[3 + i];
        }
        
        Serial.print(F("Valid 7-byte UID: "));
        for (int i = 0; i < 7; i++) {
            if (uidBuffer[i] < 0x10) Serial.print("0");
            Serial.print(uidBuffer[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
        
        return 7;
    }
    else if (rawLength == 10) {
        // Kiểm tra UID 10-byte (hiếm gặp)
        if (!validate10ByteUID(&response[3])) {
            Serial.println(F("Invalid 10-byte UID"));
            return -3;
        }
        
        // Copy UID vào buffer đầu ra
        for (int i = 0; i < 10; i++) {
            uidBuffer[i] = response[3 + i];
        }
        
        Serial.println(F("Valid 10-byte UID (truncated to first 8 bytes in buffer)"));
        return 10;
    }
    return -3;
}

/**
 * Kiểm tra UID 4-byte
 */
bool validate4ByteUID(uint8_t *uid) {
    // Byte đầu không được là 0x00 hoặc 0x88
    if (uid[0] == 0x00 || uid[0] == 0x88) {
        Serial.print(F("Invalid first byte: 0x"));
        if (uid[0] < 0x10) Serial.print("0");
        Serial.println(uid[0], HEX);
        return false;
    }
    
    // Kiểm tra không phải toàn 0x00
    bool allZero = true;
    for (int i = 0; i < 4; i++) {
        if (uid[i] != 0x00) {
            allZero = false;
            break;
        }
    }
    if (allZero) {
        Serial.println(F("UID is all zeros"));
        return false;
    }
    
    // Kiểm tra không phải toàn 0xFF
    bool allFF = true;
    for (int i = 0; i < 4; i++) {
        if (uid[i] != 0xFF) {
            allFF = false;
            break;
        }
    }
    if (allFF) {
        Serial.println(F("UID is all FFs"));
        return false;
    }
    
    // Kiểm tra Manufacturer ID hợp lệ (tùy chọn)
    Serial.print(F("Manufacturer ID: 0x"));
    if (uid[0] < 0x10) Serial.print("0");
    Serial.println(uid[0], HEX);
    
    return true;
}

/**
 * Kiểm tra UID 7-byte
 */
bool validate7ByteUID(uint8_t *uid) {
    // Byte thứ 4 (index 3) phải là 0x88 (CT flag) cho thẻ 7-byte
    if (uid[3] != 0x88) {
        Serial.print(F("Byte 4 should be 0x88, got: 0x"));
        if (uid[3] < 0x10) Serial.print("0");
        Serial.println(uid[3], HEX);
        return false;
    }
    
    // 3 byte cuối (byte 5-7) không được toàn 0x00
    bool last3AllZero = true;
    for (int i = 4; i < 7; i++) {
        if (uid[i] != 0x00) {
            last3AllZero = false;
            break;
        }
    }
    if (last3AllZero) {
        Serial.println(F("Last 3 bytes are all zeros"));
        return false;
    }
    
    // 3 byte cuối không được toàn 0xFF
    bool last3AllFF = true;
    for (int i = 4; i < 7; i++) {
        if (uid[i] != 0xFF) {
            last3AllFF = false;
            break;
        }
    }
    if (last3AllFF) {
        Serial.println(F("Last 3 bytes are all FFs"));
        return false;
    }
    
    // Byte đầu không được 0x00
    if (uid[0] == 0x00) {
        Serial.println(F("First byte is 0x00"));
        return false;
    }
    
    return true;
}

/**
 * Kiểm tra UID 10-byte (hiếm gặp, xử lý cơ bản)
 */
bool validate10ByteUID(uint8_t *uid) {
    // Kiểm tra không phải toàn 0x00
    bool allZero = true;
    for (int i = 0; i < 10; i++) {
        if (uid[i] != 0x00) {
            allZero = false;
            break;
        }
    }
    if (allZero) return false;
    
    // Kiểm tra không phải toàn 0xFF
    bool allFF = true;
    for (int i = 0; i < 10; i++) {
        if (uid[i] != 0xFF) {
            allFF = false;
            break;
        }
    }
    if (allFF) return false;
    return true;
}

void setup() {
  Serial.begin(115200);
  nfc.begin();
  Serial.println(F("PN5180 Hard-Reset..."));
  nfc.reset();
  uint8_t productVersion[2];
  nfc.readEEprom(PRODUCT_VERSION, productVersion, sizeof(productVersion));
  Serial.println(F("Product version="));
  Serial.print(productVersion[1]);
  Serial.print(".");
  Serial.println(productVersion[0]);

  if (0xff == productVersion[1]) { // if product version 255, the initialization failed
    Serial.println(F("Initialization failed!?"));
    Serial.println(F("Press reset to restart..."));
    Serial.flush();
    exit(-1); // halt
  }
  Serial.println(F("Enable RF field..."));
  nfc.setupRF();
}

uint32_t loopCnt = 0;
bool errorFlag = false;

uint8_t lastValidUID[10] = {0};
int lastValidLength = 0;
uint32_t sameCount = 0;
uint32_t diffCount = 0;

// ISO 14443 loop
void loop() {
    static uint32_t loopCnt = 0;
    
    Serial.println(F("----------------------------------"));
    Serial.print(F("Loop #"));
    Serial.println(loopCnt++);
    
    // Clear IRQ status trước khi đọc
    nfc.clearIRQStatus(0xFFFFFFFF);
    
    // Đọc thẻ bằng hàm mới
    uint8_t uidBuffer[10] = {0};
    int8_t uidLength = readCard(uidBuffer);
    
    if (uidLength > 0) {
        // UID hợp lệ
        Serial.print(F("UID length: "));
        Serial.println(uidLength);
        
        // So sánh với lần đọc trước
        if (lastValidLength > 0) {
            bool isSame = true;
            
            // Kiểm tra độ dài có giống không
            if (uidLength != lastValidLength) {
                isSame = false;
            } else {
                // Kiểm tra từng byte UID
                for (int i = 0; i < uidLength; i++) {
                    if (uidBuffer[i] != lastValidUID[i]) {
                        isSame = false;
                        break;
                    }
                }
            }
            
            if (isSame) {
                sameCount++;
                Serial.print(F("✓ Same UID ("));
                Serial.print(sameCount);
                Serial.println(F(" times)"));
            } else {
                diffCount++;
                Serial.print(F("✗ DIFFERENT UID ("));
                Serial.print(diffCount);
                Serial.println(F(" times)"));
                
                // In cả hai UID để so sánh
                Serial.print(F("Last UID: "));
                for (int i = 0; i < lastValidLength; i++) {
                    if (lastValidUID[i] < 0x10) Serial.print("0");
                    Serial.print(lastValidUID[i], HEX);
                    Serial.print(" ");
                }
                Serial.println();
                
                Serial.print(F("This UID: "));
                for (int i = 0; i < uidLength; i++) {
                    if (uidBuffer[i] < 0x10) Serial.print("0");
                    Serial.print(uidBuffer[i], HEX);
                    Serial.print(" ");
                }
                Serial.println();
            }
        }
        
        // Lưu UID hiện tại
        memcpy(lastValidUID, uidBuffer, 10);
        lastValidLength = uidLength;
        
        // Tính checksum để kiểm tra tính nhất quán
        uint8_t checksum = 0;
        for (int i = 0; i < uidLength; i++) {
            checksum ^= uidBuffer[i]; // XOR tất cả bytes
        }
        Serial.print(F("UID Checksum (XOR): 0x"));
        if (checksum < 0x10) Serial.print("0");
        Serial.println(checksum, HEX);
        
    } else if (uidLength < 0) {
        // Lỗi
        Serial.print(F("Error reading card: "));
        Serial.println(uidLength);
        
        // Reset khi gặp lỗi
        if (uidLength == -2 || uidLength == -3) {
            Serial.println(F("Resetting after card error..."));
            nfc.reset();
            delay(50);
            nfc.setupRF();
            delay(100);
        }
    } else {
        // uidLength == 0: Không có thẻ
        Serial.println(F("No card detected"));
    }
    
    Serial.println();
    delay(1000);
}



void showIRQStatus(uint32_t irqStatus) {
  Serial.print(F("IRQ-Status 0x"));
  Serial.print(irqStatus, HEX);
  Serial.print(": [ ");
  if (irqStatus & (1<< 0)) Serial.print(F("RQ "));
  if (irqStatus & (1<< 1)) Serial.print(F("TX "));
  if (irqStatus & (1<< 2)) Serial.print(F("IDLE "));
  if (irqStatus & (1<< 3)) Serial.print(F("MODE_DETECTED "));
  if (irqStatus & (1<< 4)) Serial.print(F("CARD_ACTIVATED "));
  if (irqStatus & (1<< 5)) Serial.print(F("STATE_CHANGE "));
  if (irqStatus & (1<< 6)) Serial.print(F("RFOFF_DET "));
  if (irqStatus & (1<< 7)) Serial.print(F("RFON_DET "));
  if (irqStatus & (1<< 8)) Serial.print(F("TX_RFOFF "));
  if (irqStatus & (1<< 9)) Serial.print(F("TX_RFON "));
  if (irqStatus & (1<<10)) Serial.print(F("RF_ACTIVE_ERROR "));
  if (irqStatus & (1<<11)) Serial.print(F("TIMER0 "));
  if (irqStatus & (1<<12)) Serial.print(F("TIMER1 "));
  if (irqStatus & (1<<13)) Serial.print(F("TIMER2 "));
  if (irqStatus & (1<<14)) Serial.print(F("RX_SOF_DET "));
  if (irqStatus & (1<<15)) Serial.print(F("RX_SC_DET "));
  if (irqStatus & (1<<16)) Serial.print(F("TEMPSENS_ERROR "));
  if (irqStatus & (1<<17)) Serial.print(F("GENERAL_ERROR "));
  if (irqStatus & (1<<18)) Serial.print(F("HV_ERROR "));
  if (irqStatus & (1<<19)) Serial.print(F("LPCD "));
  Serial.println("]");
}
