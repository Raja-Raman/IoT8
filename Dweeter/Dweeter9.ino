
// post tracer packets to jsonplaceholder/ Dweet.io using WiFiClient
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
const char* wifi_password2 = "yyyyyy";
const char* wifi_ssid3     = "RajaMobile";
const char* wifi_password3 = "zzzzz";

ESP8266WiFiMulti wifi_multi_client;

#define USE_DWEET

// All Dweet work is now on HTTP, port 80
#ifdef USE_DWEET  
  const char * post_host = "dweet.io"; 
  const char * post_resource = "/dweet/for/raja-vz-raman"; 
  //const char * post_resource = "/dweet/quietly/for/raja-vz-raman"; 
  int post_port = 80;

  const char *  get_host = "jsonplaceholder.typicode.com";
  //const char *  get_resource = "/users";        // dumps all 10 users
  const char *  get_resource = "/users/2";       // get one user
  int get_port = 80;   
#else 
  // This URL returns your json in a neat printable format 
  const char * post_host = "jsonplaceholder.typicode.com";   
  const char * post_resource = "/users";
  int post_port = 80;

  const char *  get_host = "jsonplaceholder.typicode.com";
  //const char *  get_resource = "/users";        // dumps all 10 users
  const char *  get_resource = "/users/2";       // get one user
  int get_port = 80;  
#endif

String post_payload = "";  
String post_header="";
String get_header=""; 
long HTTP_TIMEOUT = 2000;

unsigned long PACKET_DELAY = 30000;  // 60000;
int led  = D4;  // GPIO 2,  active LOW  
bool led_status = 0;

void setup() {
    pinMode(led, OUTPUT);  
    blink();     
    init_serial();     
    init_wifi();
    init_headers();
}

void loop() {
    SERIAL_PRINTLN("Posting data to server..");   
    make_payload (); 
    int result = send_status(post_payload);   
    SERIAL_PRINT("POST Result (0=success): ");  
    SERIAL_PRINTLN(result);
    SERIAL_PRINTLN();
    
    update_counts(result);
    led_status = !led_status;
    digitalWrite(led, led_status);
    
    delay(2000);
    get_status();  // make another GET request
    delay(PACKET_DELAY);
}
//--------------------------------------- Application layer ----------------------------------------------- 
int data_counter = 0;
int errors[5] = {0,0,0,0,0};

void make_payload () {
    //post_payload.clear();
    post_payload = String("{\"Tally\": \"")+ errors[0]+" "+ 
                    errors[1]+" "+ errors[2]+" "+ 
                    errors[3]+" "+ errors[4]+  "\" , " +
                    "\"SawTooth\" : " +data_counter  +"}";
    SERIAL_PRINTLN(post_payload);  // the tally pertains to the previous packet  
}

void update_counts(int error_code)
{
    errors[error_code] = errors[error_code]+1;
    data_counter = (data_counter+1)%10;
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

//---------------------------------------- Transport layer --------------------------------------------
void init_headers() {
  post_header = String("POST ") + 
                   post_resource +
                   " HTTP/1.1\r\n" +
                   "Host: " + post_host + "\r\n" +
                   "Content-Type: application/json; charset=utf-8\r\n"+
                   "Connection: close\r\n";

  get_header = String("GET ") + 
                   get_resource +
                   " HTTP/1.1\r\n" +
                   "Host: " + get_host + "\r\n" +
                   "Connection: close\r\n\r\n";                   
}

int send_status (String payload) {
    if(!check_wifi())
        return 1;   
    SERIAL_PRINT("Posting to: ");
    SERIAL_PRINTLN(post_host);
    String final_header = String("Content-length: ")+ payload.length()+ "\r\n\r\n";    
    
    WiFiClient wifi_client;
    wifi_client.setTimeout (HTTP_TIMEOUT);
    wifi_client.setNoDelay(true);  // disable Nagle algorithm
    bool connected = wifi_client.connect(post_host, post_port);
    if (!connected) {
        SERIAL_PRINTLN ("Count not connect to server");
        wifi_client.stop();
        return 2;
    }
    wifi_client.print(post_header);
    wifi_client.print(final_header); // this has the blank line also
    wifi_client.print(payload);

    print_response(wifi_client);

    SERIAL_PRINTLN("Ending HTTP connection..");
    SERIAL_PRINTLN("");
    wifi_client.stop();   // close connection
    return 0;  // success
}

void get_status_basic() {
    if (!check_wifi())     
        return;   
    SERIAL_PRINT("WiFi Client: Retrieving from: ");
    SERIAL_PRINTLN(get_host);
    //SERIAL_PRINTLN (get_header);

    WiFiClient wifi_client;
    wifi_client.setTimeout (HTTP_TIMEOUT);    
    wifi_client.setNoDelay(true);  // disable Nagle algorithm
    bool connected = wifi_client.connect(get_host, get_port);
    if (!connected) {
        SERIAL_PRINTLN ("Count not connect to server");
        wifi_client.stop();
        return;
    }
    wifi_client.print(get_header);
    
    basic_print_response(wifi_client);
    
    SERIAL_PRINTLN("Ending HTTP connection..");
    SERIAL_PRINTLN("");
    wifi_client.stop();   // close connection
}

void get_status() {
    if (!check_wifi())     
        return;  
    SERIAL_PRINT("WiFi Client: Retrieving from: ");
    SERIAL_PRINTLN(get_host);
    //SERIAL_PRINTLN (get_header);
    
    WiFiClient wifi_client; // Note: it is a local variable
    wifi_client.setTimeout (HTTP_TIMEOUT);    
    wifi_client.setNoDelay(true);  // disable Nagle algorithm
    bool connected = wifi_client.connect(get_host, get_port);
    if (!connected) {
        SERIAL_PRINTLN ("Count not connect to server");
        wifi_client.stop();
        return;
    }
    wifi_client.print(get_header);
    
    advanced_print_response(wifi_client);

    SERIAL_PRINTLN("Ending HTTP connection..");
    SERIAL_PRINTLN("");
    wifi_client.stop();   // close connection
}

void basic_print_response (WiFiClient wifi_client) {
    SERIAL_PRINTLN("------- Reply from the server [1]: ------");
    while (wifi_client.connected()) {
        if (wifi_client.available()) {
            String line = wifi_client.readStringUntil('\n');
            SERIAL_PRINTLN (line);
        }
    }
}

void print_response(WiFiClient wifi_client) {
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
                SERIAL_PRINTLN (response_code); // the newline is gone..
                //SERIAL_PRINTLN();               // ... so put it back
            } 
            else
            if (!eoh_found && line.length() < 2)  // skip the headers
                eoh_found = true;  
            if (eoh_found)
                SERIAL_PRINTLN (line);
        }
    }
}

void advanced_print_response(WiFiClient wifi_client) {
    // TODO: time out
    SERIAL_PRINTLN ("------- Reply from the server[3]: ------"); 
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
}

//--------------------------------------- WiFi zone ---------------------------------------------------------

bool check_wifi() {
     if(WiFi.status() == WL_CONNECTED) 
          return (true);
     SERIAL_PRINTLN("No wifi connection !");
     return (reconnect_wifi());  // this is very much needed for WifiMulti!
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
//--------------------------------------- Hardware zone ---------------------------------------------------------

void init_serial() {
    Serial.begin (115200);
    Serial.setDebugOutput(true);
    SERIAL_PRINTLN ("\n\n********************** Dweeter 8266 starting...*********************\n\n");
    delay(10); 
}    

void blink() {
    for (int i=0; i<8; i++) {
      digitalWrite(led, LOW);
      delay(100); 
      digitalWrite(led, HIGH);
      delay(100); 
    }  
}

