
// post status to jsonplaceholder and Dweet.io
// uses WifiMulti
// see it at https://dweet.io/follow/raja-vz-raman  

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

const char* wifi_ssid1     = "RajaCell";
const char* wifi_password1 = "dddd";
const char* wifi_ssid2     = "RajaACT";
const char* wifi_password2 = "eeee";
const char* wifi_ssid3     = "RajaMobile";
const char* wifi_password3 = "fffff";

ESP8266WiFiMulti wifi_multi_client;

#define USE_DWEET

#ifdef USE_DWEET
  // plots a ramp on Dweet
  const char * url = "https://dweet.io/dweet/for/raja-vz-raman";     // returns verbose response json 
  //const char * url = "https://dweet.io/dweet/quietly/for/raja-vz-raman";  // silent mode, returns 204 
  const char * thumb_print = "4a4e39710b7dd795d23506c1c4bf488d925e3fed";
  //const char *  get_url = "https://dweet.io/follow/raja-vz-raman"; 
  const char *  get_url = "http://example.com"; 
  bool use_thumb_print = true;
#else 
  // This URL returns your json in a neat printable format 
  const char * url = "http://jsonplaceholder.typicode.com/users";   
  const char * thumb_print ="dummy";
  //const char *  get_url = "http://jsonplaceholder.typicode.com/users";   // dumps all 10 users
  const char *  get_url = "http://jsonplaceholder.typicode.com/users/3";   // get one user
  bool use_thumb_print = false;
#endif

int data_counter = 0;
int led  = D4;  // GPIO 2,  active LOW  
bool led_status = 0;
unsigned long PACKET_DELAY = 15000;  // 30000;

void setup() {
    Serial.begin (115200);
    Serial.setDebugOutput(true);
    Serial.println ("\n\n**********************Dweeter 8266 starting...*********************\n\n");
    delay(10); 
    pinMode(led, OUTPUT);  
    blink();      
    init_wifi();
}

// TODO: crashes if net connection is plugged out; reset WDT, replace String with sprintf
int errors[5] = {0,0,0,0,0};
void loop() {
    String payload = String("{\"Tally\": \"")+ errors[0]+" "+ errors[1]+" "+ 
                    errors[2]+" "+ errors[3]+" "+ errors[4]+  "\" , " +
                    "\"SawTooth\" : " +data_counter  +"}";
    Serial.println(payload);
    int result = send_status(payload.c_str());  // make a POST request
    Serial.print("Result: ");
    Serial.println(result);
    ESP.wdtFeed();
    errors[result] = errors[result]+1;
    yield(); 
    data_counter = (data_counter+1)%10;
    led_status = !led_status;
    digitalWrite(led, led_status);
    delay(3000);
    get_status();  // make another GET request
    delay(PACKET_DELAY);
}

bool reconnect_wifi() {
    Serial.println("Trying to reconnect to WifiMulti..");
    WiFi.mode(WIFI_STA);
    bool wifi_connected = (wifi_multi_client.run()==WL_CONNECTED); // this is needed !!
    Serial.print("Wifi connected? : "); 
    Serial.println(wifi_connected);      
}

bool init_wifi() {
    WiFi.mode(WIFI_OFF);  // Prevents reconnection issue (taking too long to connect) *****
    delay(1000);
    // it is important to set STA mode: https://github.com/knolleary/pubsubclient/issues/138
    WiFi.mode(WIFI_STA); 
    //wifi_set_sleep_type(NONE_SLEEP_T);  // revisit & understand this
    //wifi_set_sleep_type(LIGHT_SLEEP_T); 

    Serial.println("Connecting to SSIDs:");  
    wifi_multi_client.addAP(wifi_ssid1, wifi_password1);
    Serial.println(wifi_ssid1);
    if (strlen(wifi_ssid2) > 0) {
        wifi_multi_client.addAP(wifi_ssid2, wifi_password2); 
        Serial.println(wifi_ssid2);
    }
    if (strlen(wifi_ssid3) > 0) {
        wifi_multi_client.addAP(wifi_ssid3, wifi_password3); 
        Serial.println(wifi_ssid3);
    }
            
    int MAX_ATTEMPTS = 40;  // 10 sec
    int attempts = 0;
    while(wifi_multi_client.run() != WL_CONNECTED) {   
        delay(500);
        Serial.print ("."); 
        attempts++;
        if (attempts >= MAX_ATTEMPTS) {
            Serial.println ("!\n- Could not connect to Wi-Fi -"); 
            return false;
        }
    }
    Serial.println("WiFi connected");  
    return true;
    /***
    IPAddress ip(192,168,1,100);
    IPAddress gateway(192,168,1,1);
    IPAddress subnet(255,255,255,0);
    WiFi.config(ip,gateway,subnet);
    ***/    
}

int send_status (const char* payload) {
    if(WiFi.status()!= WL_CONNECTED) {
        Serial.println("No wifi connection. Trying again..");
        reconnect_wifi();  // this is very much needed for WifiMulti!
        return 1;   
    }
    HTTPClient htclient;
    bool begun = false; // this is not useful ?!
    Serial.print("Connecting to: ");
    Serial.println(url);
    
    if (use_thumb_print)
        begun = htclient.begin (url, thumb_print);
    else
        begun = htclient.begin (url);
    
    Serial.print ("Connected to server ? : ");
    Serial.println (begun);  

    if (!begun) {
      yield();
      Serial.println ("Could not connect !");
      htclient.end();   // close connection
      return 2;
    }

    //Serial.println(payload);    
    htclient.addHeader("Content-Type", "application/json");
    int retcode = htclient.POST(payload); 
    Serial.print("Result code:");
    Serial.println(retcode);
    if (retcode < 0) {
      yield();
      ESP.wdtFeed();
      Serial.println ("Could not POST data !");
      htclient.end();   // close connection
      return 3;
    }
    if (retcode < 200 || retcode >= 300) {
      yield();
      Serial.println ("HTTP error !");
      htclient.end();   // close connection
      return 4;
    }    
    String response = htclient.getString(); 
    Serial.println("Response:");
    Serial.println(response);
    Serial.println("Client disconnected.");
    Serial.println("");
    htclient.end();   // close connection
    return 0;
}

void get_status() {
    if(WiFi.status()!= WL_CONNECTED) {
        Serial.println("No wifi connection !");  
        reconnect_wifi();  // this is very much needed for WifiMulti!        
        return;   
    }
    HTTPClient htclient;
    bool begun = false; // this is not useful ?!
    Serial.print("Retrieving from: ");
    Serial.println(get_url);
    begun = htclient.begin (get_url);
    //begun = htclient.begin (get_url, thumb_print);
    
    Serial.print ("Connected to server ? : ");
    Serial.println (begun);  
    
    int retcode = htclient.GET(); 
    Serial.print("Result code:");
    Serial.println(retcode);
    String response = htclient.getString(); 
    Serial.println("Response:");
    Serial.println(response);
    Serial.println("Client disconnected.");
    Serial.println("");
    htclient.end();   // close connection
}

void blink() {
    for (int i=0; i<8; i++) {
      digitalWrite(led, LOW);
      delay(100); 
      digitalWrite(led, HIGH);
      delay(100); 
    }  
}

