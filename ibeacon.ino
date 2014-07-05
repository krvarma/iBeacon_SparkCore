#include <math.h>

#define GAP_PROFILE_CENTRAL           0x08
#define KEYLEN                        16

#define BUILD_UINT16(loByte, hiByte) ((uint16_t)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))

#define DEVICE_INITIALIZED 0x600
#define DEVICE_DISCOVERY_DONE 0x601
#define DEVICE_INFORMATION 0x60D

static uint8_t gapCentralRoleTaskId = 0;
static uint8_t  gapCentralRoleIRK[KEYLEN] = {0};
static uint8_t  gapCentralRoleSRK[KEYLEN] = {0};
static uint32_t gapCentralRoleSignCounter = 1;
static uint8_t  gapCentralRoleMaxScanRes = 5;

uint8_t buf[64];
char szInfo[63];

// publish the message
void publish(char* event, char* message){
    Spark.publish(event, message);
}

// publish the message
void publish(char* message){
    publish("bninfo", message);
}

void setup(){Serial1.begin(57600);
    // Initialize hci
    hci_init();
    
    // Initialize LED Pins
    pinMode(D7, OUTPUT);
    pinMode(D2, OUTPUT);
}

void loop(){ 
    if(Serial1.available()){
        ble_event_process();
    }
}

// BLE Event Processing
byte ble_event_process(){
    uint8_t type, event_code, data_len, status1;
    uint16_t event;

    type = Serial1.read();
    delay(100);
    event_code = Serial1.read();
    data_len = Serial1.read();
  
    for (int i = 0; i < data_len; i++)
        buf[i] = Serial1.read();
    
    event = BUILD_UINT16(buf[0], buf[1]);
    status1 = buf[2];
  
    switch(event){
        case DEVICE_INITIALIZED:{
            hci_start_discovery();
            
            break;
        }
        case DEVICE_DISCOVERY_DONE:{
           hci_start_discovery();
      
            break;
        }
        case DEVICE_INFORMATION:{
            // Just some visual indication
            digitalWrite(D7, HIGH);
            
            // Get RSSI and Measured Power
            int rssi = buf[11];
            int txpower = buf[42];
            
            // Calculate Distance
            // This is based on the algorithm from http://stackoverflow.com/questions/20416218/understanding-ibeacon-distancing
            //
            double distance = 0.0;
            double ratio = (256 - rssi) * 1.0 / (256 - txpower);
              
            if(ratio < 1.0)
                distance = pow(ratio, 10);
            else
                distance = (0.89976)*pow(ratio,7.7095) + 0.111;
                
            
            // Publish information, since we can only have 63 chars, let's do it in two step
            // First publish the iBeacon UUID
            sprintf(szInfo, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                buf[22], buf[23], buf[24], buf[25],
                buf[26], buf[27],
                buf[28], buf[29],
                buf[30], buf[31],
                buf[32], buf[33], buf[34], buf[35], buf[36], buf[37]);
            
            publish("bnuuid", szInfo);
            
            // Delay another publish
            delay(1000);
            
            // Publish iBeacon information
            sprintf(szInfo, "Major: %d, Minor: %d, Measured Power: %d, Distance: %.2f",
                BUILD_UINT16(buf[39], buf[38]),
                BUILD_UINT16(buf[41], buf[40]),
                buf[42],
                distance);
                
            publish("bninfo", szInfo);
            
            // If the distance is less that 0.5m, that is too close, then turn on the LED on D2
            if(distance < 0.5){
                digitalWrite(D2, HIGH);
            }
            else{
                digitalWrite(D2, LOW);
            }
            
            // Blink D7 LED to indicate we found iBeacon
            digitalWrite(D7, LOW);

            break;
        }
    }
}

int hci_init(){
    return GAP_DeviceInit(gapCentralRoleTaskId, GAP_PROFILE_CENTRAL, gapCentralRoleMaxScanRes, gapCentralRoleIRK, gapCentralRoleSRK, &gapCentralRoleSignCounter);
}

int hci_start_discovery(){
    return GAP_DeviceDiscoveryRequest();
}

// Send initialize HCI command
int GAP_DeviceInit(uint8_t taskID, uint8_t profileRole, uint8_t maxScanResponses, uint8_t *pIRK, uint8_t *pSRK, uint32_t *pSignCounter){
    uint8_t len = 0;
    
    buf[len++] = 0x01;                  // -Type    : 0x01 (Command)
    buf[len++] = 0x00;                  // -Opcode  : 0xFE00 (GAP_DeviceInit)
    buf[len++] = 0xFE;
  
    buf[len++] = 0x26;                  // -Data Length
    buf[len++] = profileRole;           //  Profile Role
    buf[len++] = maxScanResponses;      //  MaxScanRsps
    memcpy(&buf[len], pIRK, 16);        //  IRK
    len += 16;
    memcpy(&buf[len], pSRK, 16);        //  SRK
    len += 16;
    memcpy(&buf[len], pSignCounter, 4); //  SignCounter
    len += 4;

    Serial1.write(buf, len);

    return 1;
}

// Send start discovery request
int GAP_DeviceDiscoveryRequest(){
    uint8_t len = 0;
    
    buf[len++] = 0x01;                 // -Type    : 0x01 (Command)
    buf[len++] = 0x04;                 // -Opcode  : 0xFE04 (GAP_DeviceDiscoveryRequest)
    buf[len++] = 0xFE;
        
    buf[len++] = 0x03;                 // -Data Length
    buf[len++] = 0x03;                 //  Mode
    buf[len++] = 0x01;                 //  ActiveScan
    buf[len++] = 0x00;                 //  WhiteList
  
    Serial1.write(buf, len);
  
    return 1;
}