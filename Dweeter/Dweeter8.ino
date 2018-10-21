
// post tracer packets to jsonplaceholder/ Dweet.io using WiFiClient
// see it at https://dweet.io/follow/raja-vz-raman  

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#define  SERIAL_PRINT(x)       Serial.print(x)
#define  SERIAL_PRINTLN(x)     Serial.println(x)
#define  SERIAL_PRINTLNF(x,y)  Serial.println(x,y) 
  
const char* wifi_ssid1     = "RajaCell";
const char* wifi_password1 = "dddddd";
const char* wifi_ssid2     = "RajaACT";
const char* wifi_password2 = "eeeeee";
const char* wifi_ssid3     = "RajaMobile";
const char* wifi_password3 = "ffffff";

ESP8266WiFiMulti wifi_multi_client;

#define USE_DWEET

// All Dweet work is now on HTTP, port 80
#ifdef USE_DWEET  
  const char * post_host = "dweet.io"; 
  const char * post_resource = "/dweet/for/raja-vz-raman"; 
  //const char * post_resource = "/dweet/quietly/for/raja-vz-raman"; 
  int post_port = 80;
#else 
  // This URL returns your json in a neat printable format 
  const char * post_host = "jsonplaceholder.typicode.com";   
  const char * post_resource = "/users";
  int post_port = 80;
#endif
  
const char *  get_host = "jsonplaceholder.typicode.com";
//const char *  get_resource = "/users";        // dumps all 10 users
const char *  get_resource = "/users/2";       // get one user
int get_port = 80;
 
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
    SERIAL_PRINTLN("Posting data to server..");                
    SERIAL_PRINTLN(payload);  // this pertains to the previous packet
    int result = send_status(payload.c_str());  //TODO: send the String object itself
    
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
    SERIAL_PRINTLN(post_host);
    String post_command = String("POST ") + 
                         post_resource +
                         " HTTP/1.1\r\n" +
                         "Host: " + post_host + "\r\n" +
                         "Content-Type: application/json; charset=utf-8\r\n"+
                         "Content-length: "+
                         strlen(payload)+ "\r\n" +
                         "Connection: close\r\n" +
                         "\r\n" +
                         payload;             
    SERIAL_PRINTLN (post_command);
    
    WiFiClient wifi_client;
    wifi_client.setNoDelay(true);  // disable Nagle algorithm
    bool connected = wifi_client.connect(post_host, post_port);
    if (!connected) {
        SERIAL_PRINTLN ("Count not connect to server");
        wifi_client.stop();
        return 2;
    }
    wifi_client.print(post_command);

    // TODO: time out
    SERIAL_PRINTLN("------- Reply from the server: ------");
    /*****
    // basic handling:
    while (wifi_client.connected()) {
        if (wifi_client.available()) {
            String line = wifi_client.readStringUntil('\n');
            SERIAL_PRINTLN (line);
        }
    }
    *****/
    bool eoh_found = false;
    bool response_code_found = false;
    String response_code;
    while (wifi_client.connected()) {
        if (wifi_client.available()) {
            String line = wifi_client.readStringUntil('\n');
            if (!response_code_found) {
                response_code_found =true;
                response_code = line;
                SERIAL_PRINT ("HTTP Response code: ");
                SERIAL_PRINTLN (response_code);
            } 
            else
            if (!eoh_found && line.length() < 2)  // skip the headers
                eoh_found = true;  
            if (eoh_found)
                SERIAL_PRINTLN (line);
        }
    }
    SERIAL_PRINTLN("Ending HTTP connection..");
    SERIAL_PRINTLN("");
    wifi_client.stop();   // close connection
    return 0;  // success
}

void get_status_basic() {
    if(WiFi.status()!= WL_CONNECTED) {
        SERIAL_PRINTLN("No wifi connection !");  
        reconnect_wifi();  // this is very much needed for WifiMulti!        
        return;   
    }
    SERIAL_PRINT("WiFi Client: Retrieving from: ");
    SERIAL_PRINTLN(get_host);
    String get_command = String("GET ") + 
                         get_resource +
                         " HTTP/1.1\r\n" +
                         "Host: " + get_host + "\r\n" +
                         "Connection: close\r\n\r\n";
    SERIAL_PRINTLN (get_command);

    WiFiClient wifi_client;
    wifi_client.setNoDelay(true);  // disable Nagle algorithm
    bool connected = wifi_client.connect(get_host, get_port);
    if (!connected) {
        SERIAL_PRINTLN ("Count not connect to server");
        wifi_client.stop();
        return;
    }
    wifi_client.print(get_command);
    // TODO: time out
    SERIAL_PRINTLN("Reply from HTTP server:");
    while (wifi_client.connected()) {
        if (wifi_client.available()) {
            String line = wifi_client.readStringUntil('\n');
            SERIAL_PRINTLN (line);
        }
    }
    SERIAL_PRINTLN("Ending HTTP connection..");
    SERIAL_PRINTLN("");
    wifi_client.stop();   // close connection
}

void get_status() {
    if(WiFi.status()!= WL_CONNECTED) {
        SERIAL_PRINTLN("No wifi connection !");  
        reconnect_wifi();  // this is very much needed for WifiMulti!        
        return;   
    }
    SERIAL_PRINT("WiFi Client: Retrieving from: ");
    SERIAL_PRINTLN(get_host);
    String get_command = String("GET ") + 
                         get_resource +
                         " HTTP/1.1\r\n" +
                         "Host: " + get_host + "\r\n" +
                         "Connection: close\r\n\r\n";
    SERIAL_PRINTLN (get_command);
    
    WiFiClient wifi_client;
    wifi_client.setNoDelay(true);  // disable Nagle algorithm
    bool connected = wifi_client.connect(get_host, get_port);
    if (!connected) {
        SERIAL_PRINTLN ("Count not connect to server");
        wifi_client.stop();
        return;
    }
    wifi_client.print(get_command);
    // TODO: time out
    SERIAL_PRINTLN ("------- Reply from the server: ------"); 
    bool eoh_found = false;
    bool response_code_found = false;
    String response_code;
    String body_content = "";
    bool PH = false;  // print headers or not
    while (wifi_client.connected()) {
        if (wifi_client.available()) {
            String line = wifi_client.readStringUntil('\n');
            if (!response_code_found) {
                response_code_found =true;
                response_code = line;
                SERIAL_PRINT ("HTTP Response code: ");
                SERIAL_PRINTLN (response_code);
                if (PH) SERIAL_PRINTLN ("------- Headers: ------");
            }
            if (!eoh_found && line.length() < 2) { // first blank line = '\n'
                eoh_found = true;
                if (PH) SERIAL_PRINT("----- end of headers. ------ ");
                if (PH) SERIAL_PRINTLN (line.length());
            }
            if (eoh_found) {
                SERIAL_PRINTLN (line);
                body_content = body_content +line +"\n" ;  // *** this could be DANGEROUS for memory ! ***
            }
            else
                if (PH) SERIAL_PRINTLN (line);
        }
    }
    if (body_content.length() > 0) {
        SERIAL_PRINTLN ("Content found, sending for parsing..");
        parse_response(body_content);
    } else {
        SERIAL_PRINTLN("<EMPTY BODY>");
    }
    SERIAL_PRINTLN("Ending HTTP connection..");
    SERIAL_PRINTLN("");
    wifi_client.stop();   // close connection
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

