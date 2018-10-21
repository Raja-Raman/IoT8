
// post status to jsonplaceholder and Dweet.io
// uses Wifi.begin()
// see it at https://dweet.io/follow/raja-vz-raman  

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
 
const char* ssid     = "RajaMobile";
const char* password = "xxxxx";

//#define USE_DWEET

#ifdef USE_DWEET
  // plots a ramp on Dweet
  const char * url = "https://dweet.io/dweet/for/raja-vz-raman";     // returns verbose response json 
  //const char * url = "https://dweet.io/dweet/quietly/for/raja-vz-raman";  // silent mode, returns 204 
  const char * thumb_print = "4a4e39710b7dd795d23506c1c4bf488d925e3fed";
  const char *  get_url = "https://dweet.io/follow/raja-vz-raman"; 
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
unsigned long PACKET_DELAY = 20000;  // 30000;

void setup() {
    Serial.begin (115200);
    Serial.setDebugOutput(true);
    Serial.println ("\n\n**********************Dweeter 8266 starting...*********************\n\n");
    delay(10); 
    pinMode(led, OUTPUT);  
    blink();      
    connect_wifi();
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
    //delay(3000);
    //get_status();  // make another GET request
    delay(PACKET_DELAY);
}

void connect_wifi() {
    WiFi.mode(WIFI_OFF);  // avoid startup delays
    delay(1000);
    WiFi.mode(WIFI_STA);
    //wifi_set_sleep_type(NONE_SLEEP_T); 
    Serial.print("Connecting to :");  
    Serial.println(ssid);  
    WiFi.begin(ssid, password);
    int timeout = 40;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);  // this should feed the WDT
      //ESP.wdtFeed();
      Serial.print(".");
      if (--timeout <= 0) break;
    }
    Serial.print("!\ntimeout :");
    Serial.println(timeout);
    Serial.print("Wifi status:");
    Serial.println(WiFi.status());
    if (WiFi.status() != WL_CONNECTED)
      Serial.println("Could not connect to WiFi");  
    else
      Serial.println("WiFi connected");  
}

int send_status (const char* payload) {
    if(WiFi.status()!= WL_CONNECTED) {
        Serial.println("No wifi connection!");
        //connect_wifi();  // this will be done automatically
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
        Serial.println("No wifi connection!");
        //connect_wifi();  // this will be done automatically
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

