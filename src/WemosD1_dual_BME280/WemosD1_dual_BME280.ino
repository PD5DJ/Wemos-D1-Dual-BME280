/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h> //https://github.com/adafruit/Adafruit_BME280_Library

// assign the ESP8266 pins to arduino pins
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15

#define BME_SCK D1
#define BME_MISO D5
#define BME_MOSI D2
#define BME1_CS D3
#define BME2_CS D4

// Replace with your network credentials
const char* ssid     = "PD5DJ-WLAN-01";
const char* password = "$aapnootmies$";

// Set web server port number to 80
WiFiServer server(80);

//Constants for BME280 input
//Adafruit_BME280 bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
Adafruit_BME280 bme1(BME1_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI
Adafruit_BME280 bme2(BME2_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI


double  BME1_temperature;
double  BME1_humidity;
double  BME1_pressure;
double  BME1_altitude;
double  BME1_hpaoffset;

double  BME2_temperature;
double  BME2_humidity;
double  BME2_pressure;
double  BME2_altitude;
double  BME2_hpaoffset;


// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output5State = "off";
String output4State = "off";

// Assign output variables to GPIO pins
const int output5 = D0;
const int output4 = D8;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);

  bme1.begin(); 
  bme2.begin(); 
  // Reads first hPa pressure to use as 0m offset.
  BME1_hpaoffset = (bme1.readPressure() / 100.0F);
  BME2_hpaoffset = (bme2.readPressure() / 100.0F);
  
  // Initialize the output variables as outputs
  pinMode(output5, OUTPUT);
  pinMode(output4, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output5, LOW);
  digitalWrite(output4, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  ReadSensors();
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /5/on") >= 0) {
              Serial.println("GPIO 5 on");
              output5State = "on";
              digitalWrite(output5, HIGH);
            } else if (header.indexOf("GET /5/off") >= 0) {
              Serial.println("GPIO 5 off");
              output5State = "off";
              digitalWrite(output5, LOW);
            } else if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("GPIO 4 on");
              output4State = "on";
              digitalWrite(output4, HIGH);
            } else if (header.indexOf("GET /4/off") >= 0) {
              Serial.println("GPIO 4 off");
              output4State = "off";
              digitalWrite(output4, LOW);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta http-equiv='refresh' content='2' name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>");
            client.println("body { background-color: #CCCCCC ;}");
            client.println(".values { font-weight: bold; font-size: 50px ; color: red; }");
            client.println("table, th { font-size: 40px ; border: 1px solid black; width: 300px; text-align:center ;}");
            client.println("table, td { font-size: 30px ; border: 1px solid black; width: 300px; text-align:center ;}");
            client.println("table { border-collapse: collapse; background-color: #bde9ba; width: 600px;}");
            client.println("h1 { font-size: 50px ; color: blue;  text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;");
            client.println("</style></head>");
            
            // Web Page Heading
            client.println("<body>");
            client.println("<h1>Filament Dryer Monitor</h1>");
            client.println("<center><table border='1'>");
            client.println("<tr><th>Dryer 1</th><th>Dryer 2</th><th>Power</th></tr>");
            client.println("<tr><td>Temperature:<br><br><span class='values'>" + String(BME1_temperature,1 ) + char(176) + "C</span></td><td>Temperature:<br><br><span class='values'>" + String(BME2_temperature,1 ) + char(176) + "C</span></td>");
            client.println("<td>");
            
            // Display current state, and ON/OFF buttons for GPIO 4  
            client.println("<p>Dryer 1<br>Status: " + output4State + "</p>");
            // If the output4State is off, it displays the ON button       
            if (output4State=="off") {
              client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
                        
            client.println("</td><tr>  "); 
            client.println("<tr><td>Humidity:<br><br><span class='values'>" + String(BME1_humidity,0) + " %</span></td><td>Humidity:<br><br><span class='values'>" + String(BME2_humidity,0) + " %</span></td>");
            client.println("<td>");
            
            // Display current state, and ON/OFF buttons for GPIO 5  
            client.println("<p>Dryer 2<br>Status: " + output5State + "</p>");
            // If the output5State is off, it displays the ON button       
            if (output5State=="off") {
              client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
            }             
                        
            client.println("</td></tr>"); 
            //client.println("<tr><td>Pressure:<br><span class='values'>" + String(BME1_pressure,1 ) + "hPa</span></td><td>Pressure:<br><span class='values'>" + String(BME2_pressure,1 ) + "hPa</span></td><td></td>");
            //client.println("<tr><td>Altitude:<br><span class='values'>" + String(BME1_altitude,1 ) + "m</span></td><td>Altitude:<br><span class='values'>" + String(BME2_altitude,1 ) + "m</span></td><td></td>");
            client.println("</table></center>");
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

void ReadSensors(){
  BME1_temperature  = bme1.readTemperature();
  BME1_pressure     = (bme1.readPressure() / 100.0F);
  BME1_altitude     = (bme1.readAltitude(BME1_hpaoffset));
  BME1_humidity     = bme1.readHumidity();
  
  BME2_temperature  = bme2.readTemperature();
  BME2_pressure     = (bme2.readPressure() / 100.0F);
  BME2_altitude     = (bme2.readAltitude(BME2_hpaoffset));
  BME2_humidity     = bme2.readHumidity();
}
