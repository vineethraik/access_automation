#include <Adafruit_Fingerprint.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiServer.h>
#include <WebSocketsServer.h>
#include <EEPROM.h>
#include <HTTPClient.h>
#include <vector>
#include <functional>
#include <ArduinoJson.h>
#include "htmlData.h"
#include "credentials.h"
#include "timer.h"

#define SOCKET_SERVER_PORT 81

struct Store
{
    uint8_t time_counter[64];
    bool modelMap[64];
    String data;

};

enum OS_STATE
{
    OS_STATE_IDLE = 0,
    OS_STATE_ENROLL = 1,
    OS_STATE_VERIFY = 2,
    OS_STATE_REGISTER = 3
};

class AccessOS
{
private:
    Adafruit_Fingerprint fp = Adafruit_Fingerprint(&Serial2);
    String templateStr;
    int templateId = -1;
    Store store;
    WebServer server;
    WebSocketsServer socketServer = WebSocketsServer(SOCKET_SERVER_PORT);
    DNSServer DNS;
    // server methods
    void readMemory();
    void writeMemory();
    void initFP();
    void initMemory();
    void initWiFi();
    void initWeb();
    void initSockets();
    void initGPIO();
    void handleButton(int, bool *, int *, int, int);
    void handleButtons();
    void handleWebServer();
    void handleWebSockets();

public:
    int osState = OS_STATE_IDLE;
    bool button_0 = false;
    int button_0_press_count = 0;
    void init();
    void handle();
    void FPdeviceInfo();
    void registerNewID(function<void(String)>);
    // void registerNewID( function<void(int)> );
    void clearTemplates();
    void clearTemplateData()
    {
        templateStr = "";
        templateId = -1;
    };

} accessOS;

void AccessOS::initWiFi()
{
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi....");
    while (WiFi.status() != WL_CONNECTED)
    {
        timer.delay(100);
        Serial.print(".");
    }

    Serial.print("SSID: " + String(WIFI_SSID) + " Password: " + String(WIFI_PASSWORD));
}

void AccessOS::clearTemplates()
{
    uint8_t code = fp.emptyDatabase();
    if (code == FINGERPRINT_OK)
    {
        Serial.println("Database cleared");
    }
    else
    {
        Serial.println("Database clear failed");
    }

    templateStr = "";
    templateId = -1;
    for (size_t i = 0; i < sizeof(store.time_counter) / sizeof(store.time_counter[0]); i++)
    {
        store.time_counter[i] = 0;
        store.modelMap[i] = false;
    }
    writeMemory();
};

void AccessOS::readMemory()
{
    if (EEPROM.read(0) == 108)
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
        EEPROM.write(0, 108);
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
    initWiFi();
    initGPIO();
    initWeb();
    initSockets();

    JsonDocument doc;
    String test = "";

    JsonArray dataArray = doc["data"].to<JsonArray>();

    JsonObject obj1 = dataArray.add<JsonObject>();
    obj1["name"] = "1234567890123456789012345";
    obj1["usn"] = "4sf18ec124";
    obj1["FID"] = 64;

    JsonObject obj2 = dataArray.add<JsonObject>();
    obj2["name"] = "1234567890123456789012346";
    obj2["usn"] = "4sf18ec056";
    obj2["FID"] = 63;

    serializeJson(doc, test);
    Serial.println("JSON Document with array of objects:");
    Serial.println(test);
    Serial.println("Length of JSON string: " + String(test.length()));

    // Test searching the array for a specific key
    Serial.println("\nSearching for FID 64 in the array:");
    for (JsonObject obj : dataArray) {
        if (obj["FID"] == 64) {
            Serial.println("Found FID 64!");
            Serial.print("Details: ");
            String foundData;
            serializeJson(obj, foundData);
            Serial.println(foundData);
            break;
        }
    }

    // clearTemplates();
}

void AccessOS::initGPIO()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(0, INPUT);
}

void AccessOS::initMemory()
{
    store.data.reserve(5120);
    EEPROM.begin(6144);
    accessOS.readMemory();
}

void AccessOS::initFP()
{
    fp.begin(57600);
    // fp.LEDcontrol(FINGERPRINT_LED_OFF);
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

void AccessOS::handleButton(int pin, bool *state, int *counter, int debounceTime = 50, int pressCountDetachTime = 1000)
{
    bool button_0_state = !digitalRead(pin);
    unsigned int now = millis();
    static unsigned int past_0 = millis();
    static unsigned int press_count_0_past = millis();
    static int press_count_0 = 0;

    if (button_0_state != (*state))
    {
        if (now - past_0 > debounceTime)
        {
            past_0 = millis();
            if ((*state) == false)
            {
                press_count_0++;
                press_count_0_past = millis();
            }
            (*state) = button_0_state;
        }
    }
    else
    {
        past_0 = millis();
    }
    if (millis() - press_count_0_past > pressCountDetachTime)
    {
        (*counter) = press_count_0;
        press_count_0 = 0;
    }
};

void AccessOS::handleButtons()
{
    handleButton(0, &accessOS.button_0, &accessOS.button_0_press_count);
}

void AccessOS::handle()
{
    handleButtons();
    handleWebServer();
    handleWebSockets();
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
        Serial.println("Unknown error");
    }

    p = fp.fingerFastSearch();
    if (p == FINGERPRINT_OK)
    {
        Serial.printf("Found a print match at slot #%d!\n", fp.fingerID);
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
        return;
    }
    else if (p == FINGERPRINT_NOTFOUND)
    {
        Serial.println("Did not find a match");
        return;
    }
    else
    {
        Serial.println("Unknown error");
        return;
    }
}

void AccessOS::FPdeviceInfo()
{
    Serial.print("Capacity:");
    Serial.println(fp.capacity);
    Serial.print("Template count:");
    fp.getTemplateCount();
    Serial.println(fp.templateCount);
}

void AccessOS::registerNewID(function<void(String)> _updateFuction = nullptr)
{
    auto updateFunction = [_updateFuction](int progress = 0, String status = "", String message = "", int templateId = -1)
    {if(_updateFuction!=nullptr){JsonDocument res; res["progress"] = progress; res["status"] = status;
            res["message"] = message;
            String data;
            serializeJson(res, data);
            _updateFuction(data);
        } };

    osState = OS_STATE_REGISTER;
    static bool runLED = true;
    Serial.println("Registering new ID");
    updateFunction(0, "Registering new ID");

    // start led blinking event (unchanged)
    timer.set_new_event(0, 200, []()
                        {
    static bool ledState = false;
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, runLED ? ledState : false);
    return runLED; });

    int maxSlots = 64;
    int captureSlot = 0;
    int newTemplateId = -1;
    int failCount = 0;
    int failLimit = 100;

    captureSlot = 0;
    while (true)
    {
        // Serial.printf("Capturing slot: %d , Fail count: %d\n", captureSlot, failCount);
        // captureSlot = fp.getTemplateCount();
        failCount++;
        if (failCount > failLimit)
        {
            Serial.println("Failed to register");
            return;
        }
        timer.delay(1000);
        uint8_t res = fp.getImage();
        if (res != FINGERPRINT_OK)
            continue;
        res = fp.image2Tz((captureSlot % 2) + 1);
        if (res != FINGERPRINT_OK)
            continue;
        res = fp.fingerFastSearch();
        if (res == FINGERPRINT_OK)
        {
            if ((fp.fingerID != 1) && (fp.fingerID != 2) && (fp.fingerID != 3) && (fp.fingerID != 4))
            {
                Serial.println("Found a print match, not  a new credential, returning to idle");

                osState = OS_STATE_IDLE;
                runLED = false;
                return;
            }
        }

        updateFunction(captureSlot * 25, String((captureSlot+1) * 25) + "\% completed", "update function test");
        if (captureSlot == 1 || captureSlot == 3)
        {
            Serial.println("captures is 1 or 3");

            res = fp.createModel();
            if (res != FINGERPRINT_OK)
            {
                Serial.println("Error creating model");
                captureSlot = 0;
                continue;
            }
            fp.storeModel(captureSlot == 1 ? 3 : 4);
        }
        if (captureSlot == 3)
        {
            Serial.println("captures is  3");

            fp.loadModel(3);
            fp.storeModel(1);
            fp.loadModel(4);
            fp.storeModel(2);
            res = fp.createModel();
            if (res != FINGERPRINT_OK)
            {
                Serial.println("Error creating model");
                captureSlot = 0;
                continue;
            }
            fp.storeModel(1);
            newTemplateId = 1;
        }

        if (newTemplateId == 1)
        {
            Serial.println("template is 1");

            for (size_t i = 0; i < maxSlots - 4; i++)
            {
                if (store.modelMap[maxSlots - i] == false)
                {
                    store.modelMap[maxSlots - i] = true;
                    store.time_counter[maxSlots - i] = 1;
                    newTemplateId = maxSlots - i;
                    fp.storeModel(newTemplateId);
                    Serial.print("Registered new ID: ");
                    Serial.println(newTemplateId);
                    updateFunction(100, "completed", "update function test", newTemplateId);
                    newTemplateId = -1;
                    Serial.println("done with registration");
                    runLED = false;
                    fp.deleteModel(1);
                    fp.deleteModel(2);
                    fp.deleteModel(3);
                    fp.deleteModel(4);
                    writeMemory();

                    osState = OS_STATE_IDLE;
                    runLED = false;

                    return;
                }
            }
        }
        Serial.printf("Capturing slot: %d , Fail count: %d\n", captureSlot, failCount);

        failCount--;
        captureSlot++;
    }

    Serial.println("done with registration");
    runLED = false;

    osState = OS_STATE_IDLE;
}

// void AccessOS::registerNewID(function<void(int)> updateMethod){

// }
// web server methods
void AccessOS::initWeb()
{

    server.begin(80);
    server.onNotFound([this]()
                      { server.sendHeader("Location", "/", true); });
    server.on("/", [this]()
              { server.send(200, "text/html", indexHTML); });
    server.on("*", [this]()
              { server.send(200, "text/plain", "No things here"); });
}

void AccessOS::handleWebServer()
{
    server.handleClient();
}

// web socket methods
void AccessOS::initSockets()
{
    socketServer.begin();
    socketServer.onEvent([this](uint8_t num, WStype_t type, uint8_t *payload, size_t length)
                         {
        switch (type)
        {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = socketServer.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            }
            break;
        case WStype_TEXT:
            {
                JsonDocument doc,res;
                DeserializationError err = deserializeJson(doc, payload);
                if (err)
                {
                    Serial.print(F("deserializeJson() failed: "));
                    Serial.println(err.c_str());
                    return;
                }
                String type = doc["type"];
                Serial.println(type);
                if (type == "REGISTER_FINGERPRINT"){
                    registerNewID([this,num](String data){
                        Serial.println(data);
                        JsonDocument res,payload;
                        deserializeJson(payload,data);
                        res["type"] = "REGISTER_PROGRESS";
                        res["payload"] = payload;
                        String str;
                        serializeJson(res,str);
                        socketServer.sendTXT(num,str.c_str(),str.length());
                    });
                }else if(type == ""){

                }
            }

            break;
        case WStype_BIN:
            Serial.printf("[%u] get binary length: %u\n", num, length);
            break;
        } });
}

void AccessOS::handleWebSockets()
{
    socketServer.loop();
}
