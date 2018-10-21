
// post tracer packets to jsonplaceholder/ Dweet.io
// uses WifiMulti; rudimentarily parses HTTP GET return value.
// see it at https://dweet.io/follow/raja-vz-raman  

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#define  SERIAL_PRINT(x)       Serial.print(x)
#define  SERIAL_PRINTLN(x)     Serial.println(x)
#define  SERIAL_PRINTLNF(x,y)  Serial.println(x,y) 
  
const char* wifi_ssid1     = "RajaCell";
const char* wifi_password1 = "xxxxx";
const char* wifi_ssid2     = "RajaACT";
const char* wifi_password2 = "yyyyy";
const char* wifi_ssid3     = "RajaMobile";
const char* wifi_password3 = "zzzzz";

ESP8266WiFiMulti wifi_multi_client;

#define USE_DWEET

// All Dweet work is now on HTTP, port 80
#ifdef USE_DWEET  
  //const char * post_url = "http://dweet.io/dweet/for/raja-vz-raman";     // returns verbose 200 response  
  const char * post_url = "http://dweet.io/dweet/quietly/for/raja-vz-raman";  // silent mode, returns 204 
  //const char *  get_url = "http://dweet.io/follow/raja-vz-raman";   // too verbose
  //const char *  get_url = "http://example.com"; 
  const char *  get_url = "http://jsonplaceholder.typicode.com/users/5"; 
#else 
  // This URL returns your json in a neat printable format 
  const char * post_url = "http://jsonplaceholder.typicode.com/users";   
  //const char *  get_url = "http://jsonplaceholder.typicode.com/users";   // dumps all 10 users
  const char *  get_url = "http://jsonplaceholder.typicode.com/users/3";   // get one user
#endif

int data_counter = 0;
int led  = D4;  // GPIO 2,  active LOW  
bool led_status = 0;
unsigned long PACKET_DELAY = 30000;  // 10000;

void setup() {
    Serial.begin (115200);
    Serial.setDebugOutput(true);
    SERIAL_PRINTLN ("\n\n********************** Dweeter 8266 starting...*********************\n\n");
    delay(10); 
    pinMode(led, OUTPUT);  
    blink();      
    init_wifi();
}

// TODO: crashes if internet connection is plugged out; reset WDT, replace String with sprintf etc.
int errors[5] = {0,0,0,0,0};
void loop() {
    String payload = String("{\"Tally\": \"")+ errors[0]+" "+ errors[1]+" "+ 
                    errors[2]+" "+ errors[3]+" "+ errors[4]+  "\" , " +
                    "\"SawTooth\" : " +data_counter  +"}";
    SERIAL_PRINTLN(payload);  // this pertains to the previous packet
    int result = send_status(payload.c_str());  // make a POST request
    SERIAL_PRINT("POST Result: ");
    SERIAL_PRINTLN(result);
    SERIAL_PRINTLN();
    errors[result] = errors[result]+1;
    data_counter = (data_counter+1)%10;
    led_status = !led_status;
    digitalWrite(led, led_status);
    //ESP.wdtFeed();     
    delay(3000);
    get_status();  // make another GET request
    delay(PACKET_DELAY);
}

bool reconnect_wifi() {
    SERIAL_PRINTLN("Trying to reconnect to WifiMulti..");
    WiFi.mode(WIFI_STA);
    int MAX_ATTEMPTS = 40;  // 10 sec
    int attempts = 0;
    bool connected = true;
    while(wifi_multi_client.run() != WL_CONNECTED) {   
        delay(500);
        SERIAL_PRINT ("."); 
        attempts++;
        if (attempts >= MAX_ATTEMPTS) {
            SERIAL_PRINTLN ("!\n- Could not reconnect to Wi-Fi -"); 
            connected = false;
        }
    }
    SERIAL_PRINT("Wifi connected? : "); 
    SERIAL_PRINTLN(connected);    
    if (connected)
        dump_wifi();  
    return(connected);
}

bool init_wifi() {
    WiFi.mode(WIFI_OFF);  // Prevents reconnection issue (taking too long to connect) *****
    delay(1000);
    // it is important to set STA mode: https://github.com/knolleary/pubsubclient/issues/138
    WiFi.mode(WIFI_STA); 
    //wifi_set_sleep_type(NONE_SLEEP_T);  // revisit & understand this
    //wifi_set_sleep_type(LIGHT_SLEEP_T); 

    SERIAL_PRINTLN("Enlisting SSIDs:");  
    wifi_multi_client.addAP(wifi_ssid1, wifi_password1);
    SERIAL_PRINTLN(wifi_ssid1);
    if (strlen(wifi_ssid2) > 0) {
        wifi_multi_client.addAP(wifi_ssid2, wifi_password2); 
        SERIAL_PRINTLN(wifi_ssid2);
    }
    if (strlen(wifi_ssid3) > 0) {
        wifi_multi_client.addAP(wifi_ssid3, wifi_password3); 
        SERIAL_PRINTLN(wifi_ssid3);
    }  
    int MAX_ATTEMPTS = 40;  // 10 sec
    int attempts = 0;
    while(wifi_multi_client.run() != WL_CONNECTED) {   
        delay(500);
        SERIAL_PRINT ("."); 
        attempts++;
        if (attempts >= MAX_ATTEMPTS) {
            SERIAL_PRINTLN ("!\n- Could not connect to Wi-Fi -"); 
            return false;
        }
    }
    SERIAL_PRINTLN("WiFi connected");  
    dump_wifi();
    return true;
    /***
    IPAddress ip(192,168,1,100);
    IPAddress gateway(192,168,1,1);
    IPAddress subnet(255,255,255,0);
    WiFi.config(ip,gateway,subnet,gateway); 
    // https://arduino.stackexchange.com/questions/55134/esp8266httpclient-conflict-with-other-libraries
    ***/    
}

void dump_wifi() {
  // print the SSID of the network you're attached to
  SERIAL_PRINT("Connected to WiFi SSID: ");
  SERIAL_PRINTLN(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  SERIAL_PRINT("IP Address: ");
  SERIAL_PRINTLN(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  SERIAL_PRINT("Signal strength (RSSI):");
  SERIAL_PRINT(rssi);
  SERIAL_PRINTLN(" dBm");
  SERIAL_PRINTLN();
}

int send_status (const char* payload) {
    if(WiFi.status()!= WL_CONNECTED) {
        SERIAL_PRINTLN("No wifi connection. Trying again..");
        reconnect_wifi();  // this is very much needed for WifiMulti!
        return 1;   
    }
    SERIAL_PRINT("Posting to: ");
    SERIAL_PRINTLN(post_url);
    //WiFiClient wifi_client;
    HTTPClient htclient;
    //bool begun = htclient.begin (wifi_client,post_url); // this just parses the URL 
    bool begun = htclient.begin (post_url); // this just parses the URL  
    if (!begun) {
      SERIAL_PRINTLN ("URL pasing error!");
      htclient.end();   // releases buffers; not needed for temporary client objects
      return 2;
    }
    //SERIAL_PRINTLN(payload);    
    htclient.addHeader("Content-Type", "application/json;charset=utf-8");
    int retcode = htclient.POST(payload); 
    SERIAL_PRINT("HTTP POST return code:");
    SERIAL_PRINTLN(retcode);
    if (retcode < 0) {
      SERIAL_PRINTLN ("Could not POST data to the net!");
      htclient.end();   // close connection
      return 3;
    }
    if (retcode < 200 || retcode >= 300) {
      SERIAL_PRINTLN ("HTTP server returned error !");
      htclient.end();   // close connection
      return 4;
    }    
    //yield(); // will it not be harmful to yield here ?
    String response = htclient.getString(); 
    if (response.length() > 0) {
        SERIAL_PRINTLN("The server says:");
        SERIAL_PRINTLN(response);
    }
    else
        SERIAL_PRINTLN ("<empty> response from the server.");
    SERIAL_PRINTLN("Ending HTTP connection..");
    //SERIAL_PRINTLN("");
    htclient.end();   // close connection
    return 0;  // success
}

void get_status() {
    if(WiFi.status()!= WL_CONNECTED) {
        SERIAL_PRINTLN("No wifi connection !");  
        reconnect_wifi();  // this is very much needed for WifiMulti!        
        return;   
    }
    HTTPClient htclient;
    bool begun = false; // this only flags parsing error
    SERIAL_PRINT("Retrieving from: ");
    SERIAL_PRINTLN(get_url);
    //SERIAL_PRINTLN("Note that this is a plain HTTP call on port 80");
    begun = htclient.begin (get_url);
    if (!begun) {
      SERIAL_PRINTLN ("URL pasing error!");
      htclient.end();   // releases buffers; not needed for temporary client objects
      return;
    }
    int retcode = htclient.GET(); 
    SERIAL_PRINT("HTTP GET return code:");
    SERIAL_PRINTLN(retcode);
    if (retcode < 0) {
      SERIAL_PRINTLN ("Could not GET data from the net!");
      htclient.end();   // close connection
      return;
    }
    if (retcode < 200 || retcode >= 300) {
      SERIAL_PRINTLN ("HTTP server returned error !");
      htclient.end();   // close connection
      return;
    } 
    String response = htclient.getString(); 
    //SERIAL_PRINTLN("Data from HTTP server:");
    //SERIAL_PRINTLN(response);
    // you can also convert into a C style string:
    //const char *response = htclient.getString().c_str();

    parse_response(response);
    
    SERIAL_PRINTLN("Ending HTTP connection..");
    SERIAL_PRINTLN("");
    htclient.end();   // close connection
}

// this is specifically written for jsonplaceholder.typicode.com
void parse_response(String response){
    int index1, index2, index3;
    String search_str = "\"name\": "; 
    index1 = response.indexOf(search_str);
    if (index1 >= 0) {
        index2 = index1 + search_str.length();
        index3 = response.indexOf("\n", index2);
        SERIAL_PRINT("Name: ");
        SERIAL_PRINTLN(response.substring(index2, index3-1));
    }
    else
        SERIAL_PRINTLN("User name not found");
            
    search_str = "\"email\": ";
    index1 = response.indexOf(search_str);
    if (index1 >= 0) {
        index2 = index1 + search_str.length();
        index3 = response.indexOf("\n", index2);
        SERIAL_PRINT("Mail: ");
        SERIAL_PRINTLN(response.substring(index2, index3-1));
    }
    else
        SERIAL_PRINTLN("Mail adderss not found");

    search_str = "\"phone\": ";
    index1 = response.indexOf(search_str);
    if (index1 >= 0) {
        index2 = index1 + search_str.length();
        index3 = response.indexOf("\n", index2);
        SERIAL_PRINT("Phone: ");
        SERIAL_PRINTLN(response.substring(index2, index3-1));
    }
    else
        SERIAL_PRINTLN("Phone not found");        
}

void blink() {
    for (int i=0; i<8; i++) {
      digitalWrite(led, LOW);
      delay(100); 
      digitalWrite(led, HIGH);
      delay(100); 
    }  
}

