// Import required libraries
#include <DNSServer.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <Ticker.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

//Dirección IP del Servidor
#define SERVER_IP "192.168.1.10:4000"

const char* http_username = "admin";
const char* http_password = "admin";
const char* PARAM_INPUT_3 = "state";
const int output = 5;
const int buttonPin = 4;

// Variables will change:
int ledState = LOW;          // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
Ticker ticker;
DNSServer dnsServer;

#ifndef LED_BUILTIN
#define LED_BUILTIN 13 // ESP32 DOES NOT DEFINE LED_BUILTIN
#endif
int LED = LED_BUILTIN;

const char* PARAM_INPUT_1 = "SSID";
const char* PARAM_INPUT_2 = "PASSWORD";
String ssid, password;
int bandera = 0, count =0 ;

// HTML web page to handle 3 input fields (ssid, password)
const char id_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Configuraci&oacute;n ID de dispositivo</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h2>Configuraci&oacute;n de ID del ESP8266</h2>
  <form action="/id">
     <p>ID: <input type="text" name="ID" placeholder="Escriba el ID asigando" required></p>
     <p><input type="submit">
     <input type="reset" value="BORRAR"></p>
     <p>Haga clic en el siguiente enlace si desea <a href="/control">Ir al control de Iluminaci&oacute;n</a>.</p>
     <p><br><a href="/">Ir a la p&aacute;gina de Configuraci&oacute;n de WIFI</a></p>
  </form>
</body></html>)rawliteral";

// HTML web page to handle 3 input fields (ssid, password)
const char wifi_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Configuraci&oacute;n acceso a red WIFI</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h2>Configuraci&oacute;n de acceso a WIFI</h2>
  <h3>Ingrese los par&aacute;metros solicitados para obtener la direcci&oacute;n IP de su red WIFI: %DIR_IP_WIFI%</h3>
  <form action="/get">
     <p>SSID: <input type="text" name="SSID" placeholder="Escriba nombre de la red WIFI" required></p>
     <p>PASSWORD: <input type="password" name="PASSWORD" placeholder="Escriba la contrase&ntilde;a" required></p>
     <p><input type="submit" onclick="mensaje('Sus par&aacute;metros se aplicar&aacute;n, en caso de estar err&oacute;neos puede volver nuevamente a este portal')" value="GUARDAR">
     <input type="reset" value="BORRAR"></p>
     <p>Haga clic en el siguiente enlace si desea <a href="/control">Ir al control de Iluminaci&oacute;n</a>.</p>
  </form>
  <script>
     function mensaje(texto) {
       alert(texto);
     }
  </script>
</body></html>)rawliteral";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Iluminaci&oacute;n Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 2.6rem;}
    p {font-size: 2.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ff0000; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>Control de Iluminaci&oacute;n Web Server</h2>
  <p>Salida - GPIO5:D1 - Estado <span id="outputState">%STATE%</span></p>
  <button onclick="logoutButton()">Cerrar Sesi&oacute;n</button>
  %BUTTONPLACEHOLDER%
  <p><h3><br><a href="/">Ir a la p&aacute;gina de Configuraci&oacute;n de WIFI</a></h3></p>
<script>function toggleCheckbox(element){
  var xhr = new XMLHttpRequest();
  if(element.checked){ 
    xhr.open("GET", "/update?state=1", true); 
  }
  else{ 
    xhr.open("GET", "/update?state=0", true);
  }
  xhr.send();
}

function logoutButton() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/logout", true);
  xhr.send();
  setTimeout(function(){ window.open("/logged-out","_self"); }, 1000);
}

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var inputChecked;
      var outputStateM;
      if( this.responseText == 1){ 
        inputChecked = true;
        outputStateM = "On";
      }
      else { 
        inputChecked = false;
        outputStateM = "Off";
      }
      document.getElementById("output").checked = inputChecked;
      document.getElementById("outputState").innerHTML = outputStateM;
    }
  };
  xhttp.open("GET", "/state", true);
  xhttp.send();
}, 1000 );
</script>
</body>
</html>
)rawliteral";

const char logout_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <h3>Sesi&oacute;n Cerrada, haga clic en el siguiente enlace si desea <a href="/control">Regresar al control de Iluminaci&oacute;n</a>.</h3>
  <h3>Sesi&oacute;n Cerrada, haga clic en el siguiente enlace si desea <a href="/">Ir al men&uacute; de configuraci&oacute;n</a>.</h3>
  <p><strong>Nota:</strong> Cierre todas las pesta&ntilde;as del navegador web para completar el proceso de cierre de sesi&oacute;n.</p>
</body>
</html>
)rawliteral";

String outputState(){
  if(digitalRead(output)){
    return "checked";
  }
  else {
    return "";
  }
  return "";
}

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    String outputStateValue = outputState();
    buttons+= "<p><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label></p>";
    return buttons;
  }

  return String();
}

// Replaces placeholderDIR_IP_WIFI with text section in your web page
String dirIP(const String& ip1){
  //Serial.println(var);
  if(ip1 == "DIR_IP_WIFI"){
    String textIP ="", IP;
    const IPAddress& IP1 = WiFi.localIP();
    IP = String(IP1[0])+"."+String(IP1[1])+"."+String(IP1[2])+"."+String(IP1[3]);
    textIP = "<h3>Direcci&oacute;n IP asignada: http://"  + IP + "</h3>";
    return textIP;
  }
  return String();
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/html", "<p><h2><br><a href=\"/\">Ir a la p&aacute;gina de Configuraci&oacute;n de WiFi</a></h2></p>");
}

void tick()
{
  //toggle state
  digitalWrite(LED, !digitalRead(LED));     // set pin to the opposite state
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(output, OUTPUT);
  digitalWrite(output, LOW);
  pinMode(buttonPin, INPUT);
  
  WiFi.softAP("UTS2021");
  dnsServer.start(53, "*", WiFi.softAPIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", wifi_html, dirIP);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage, inputMessage1;
    String inputParam, inputParam1;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)&&request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      ssid = inputMessage;
      inputMessage1 = request->getParam(PARAM_INPUT_2)->value();
      inputParam1 = PARAM_INPUT_2;
      password = inputMessage1;
      bandera = 1;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
      bandera = 0;
    }
    Serial.println("SSID: "+inputMessage);
    Serial.println("PASSWORD: "+inputMessage1);
    request->send(200, "text/html", "<h3>HTTP GET petici&oacute;n enviada al ESP8266 en los campos (" 
                                     + inputParam + ", " + inputParam1 + ") con valores: "
                                     + inputMessage + ", " + inputMessage1 +
                                     "</h3><h3><br><a href=\"/\">Regresar a la p&aacute;gina de configuraci&oacute;n para conocer su IP asignada</a></h3><h3>Haga clic en el siguiente enlace si desea <a href=\"/control\">Ir al control de Iluminaci&oacute;n</a>.</h3>");
  });

  // Route for root / web page
  server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request){
      if(!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(401);
  });

  server.on("/logged-out", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", logout_html, processor);
  });

  // Send a GET request to <ESP_IP>/update?state=<inputMessage>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
    if (request->hasParam(PARAM_INPUT_3)) {
      inputMessage = request->getParam(PARAM_INPUT_3)->value();
      inputParam = PARAM_INPUT_3;
      digitalWrite(output, inputMessage.toInt());
      ledState = !ledState;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });

  // Send a GET request to <ESP_IP>/state
  server.on("/state", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(digitalRead(output)).c_str());
  });

  server.onNotFound(notFound);

  //set led pin as output
  pinMode(LED, OUTPUT);
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);
  delay(5000);
  if(WiFi.status() == WL_CONNECTED){
    ticker.detach();
    //keep LED off
    digitalWrite(LED, HIGH);
    Serial.println("Connecting to WiFi..");
    Serial.println(WiFi.localIP());
   }  
  //Borrar credenciales de WiFi en caso estar almacenadas
  //WiFi.resetSettings();
  
  // Start server
  server.begin();
}
  
void loop() {
  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);
  
  int cont=0;
  dnsServer.processNextRequest();
  if (bandera){
    cont = 0;
    ticker.attach(0.6, tick);
    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      cont = cont + 1;
      if (cont == 20){
       bandera = 0;
       return; 
      }
      Serial.println("Connecting to WiFi..");
      if(WiFi.status() == WL_CONNECTED){
        ticker.detach();
        //keep LED off
        digitalWrite(LED, HIGH);
      }
    }
    if(WiFi.status() == WL_CONNECTED){
        ticker.detach();
        //keep LED off
        digitalWrite(LED, HIGH);
      }
    // Print ESP Local IP Address
    Serial.println(WiFi.localIP());
    bandera = 0;
   }
  
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        ledState = !ledState;

        //----------AUDITORIA EN EL SERVIDOR-------------------------------
        //Variables para GET y POST
        const char * headerkeys[] = {"User-Agent","Set-Cookie","Cookie","Date","Content-Type","Connection"} ;
        size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
        String setCookie;
        WiFiClient client;
        HTTPClient http;
        //--------ENVIAR PETICION POST-AUTENTICACION-----------------      
        Serial.print("[HTTP] begin...\n");
        // configure traged server and url 
        String url = "http://"SERVER_IP"/signin";
        http.begin(client, url); //HTTP
        Serial.print(url);
        Serial.print("\n");
        http.addHeader("Connection", "keep-alive");
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        Serial.print("[HTTP] POST...\n");
        // start connection and send HTTP header and body
        String post = "username=UTS&password=Uts2021";
        http.collectHeaders(headerkeys,headerkeyssize);
        int httpCode = http.POST(post);
        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    
          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_FOUND) {
            const String& payload = http.getString();
            Serial.println("received payload:\n<<");
            Serial.println(payload);
            Serial.println(">>");
            //--------ENVIAR PETICION GET-AUDITAR--------------------
            String IP;
            const IPAddress& IP1 = WiFi.localIP();
            IP = String(IP1[0])+"."+String(IP1[1])+"."+String(IP1[2])+"."+String(IP1[3]);
            url = "http://"SERVER_IP"/procesos/"+IP;
            setCookie = http.header("Set-Cookie").c_str();
            http.begin(url); //HTTP
            Serial.print(url);
            Serial.print("\n");
            http.addHeader("Cookie", setCookie);
            httpCode = http.GET();
            Serial.println("received payload2:\n<<");
            Serial.println(http.getString());
            Serial.println(">>");
          }
        } else {
          Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
        //----------AUDITORIA EN EL SERVIDOR-------------------------------
      }
    }
  }

  // set the LED:
  digitalWrite(output, ledState);

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
}
