#include <Adafruit_Fingerprint.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiServer.h>
#include <WebSockets.h>
#include <EEPROM.h>

struct Store
{

    
    uint8_t time_counter[64];
    bool modelMap[64];
};

class AccessOS
{
private:
    Adafruit_Fingerprint fp = Adafruit_Fingerprint(&Serial2);
    String templateStr;
    int templateId = -1;
    Store store;
    void readMemory();
    void writeMemory();
    void initFP();
    void initMemory();

public:
    void init();
    void handle();
    void FPdeviceInfo();
    void registerNewID();
    void clearTemplateData(){templateStr = "";templateId = -1;};

} accessOS;

void AccessOS::readMemory()
{
    if (EEPROM.read(0) == 109)
    {
        EEPROM.get(10, store);
    }
    else
    {
        Serial.println("First run");
        for (int i = 0; i < 64; i++)
        {
            store.time_counter[i] = 0;
            store.modelMap[i] = false;
        }
        EEPROM.put(10, store);
        EEPROM.write(0, 109);
        EEPROM.commit();
        // delay(1000);
    }
}

void AccessOS::writeMemory()
{
    EEPROM.put(10, store);
    EEPROM.commit();
}
void AccessOS::init()
{
    initFP();
    initMemory();
}

void AccessOS::initMemory()
{
    EEPROM.begin(2048);
    accessOS.readMemory();
}

void AccessOS::initFP() {
    fp.begin(57600);
    templateStr.reserve(1024);
    // fp.emptyDatabase();
    if (fp.verifyPassword())
    {
        Serial.println("Found fingerprint sensor!");
    }
    else
    {
        Serial.println("Did not find fingerprint sensor :(");
        while (1)
        {
            delay(1);
        } // Don't proceed if sensor not found
    }
}
void AccessOS::handle()
{
    uint8_t p = fp.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
    case FINGERPRINT_NOFINGER:
        return;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        return;
    default:
        Serial.println("Unknown error");
        return;
    }

    p = fp.image2Tz(1);

    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        return;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        return;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        return;
    default:
        Serial.println("Unknown error");}


    

}

void AccessOS::FPdeviceInfo()
{
    Serial.print("Capacity");
    Serial.println(fp.capacity);
    Serial.print("Template count");
    Serial.println(fp.templateCount);
}

void AccessOS::registerNewID()
{
    static bool runLED = true;
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.println("Registering new ID");
    timer.set_new_event(0,50, []() {
        static bool ledState = false;
        ledState = !ledState;
        digitalWrite(LED_BUILTIN, runLED?ledState:false);

        return runLED;
    });
    while (true)
    {
        timer.delay(1000);
        uint8_t p = fp.getImage();
        if (p != FINGERPRINT_OK)
            continue;
        p = fp.image2Tz(1);
        if (p != FINGERPRINT_OK)
            continue;   
        p = fp.fingerFastSearch();
        if (p == FINGERPRINT_OK)
           {Serial.println("Found a print match!, no new ID"+String(fp.fingerID)+":"+String(fp.confidence)); continue;}else{
            templateId=1;
           }
        // templateId = fp.fingerID;
        if(templateId!=-1){
            int index =63;
            while(true){
                if(store.modelMap[index]==false){
                    store.modelMap[index]=true;
                    store.time_counter[index] = 1;
                    writeMemory();
                    break;
                }
                index--;
            }
            fp.loadModel(templateId);
            fp.storeModel(index);
            templateId=-1;
           break; 
        }else if(templateId==-2){

        }
        
    }
    
    Serial.println("done with delay");

    runLED = false;

}