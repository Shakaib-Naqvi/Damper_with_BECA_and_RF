#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

// #define DEBUG

// Create an instance of the web server on port 80
WebServer server(80);
Preferences preferences;

String ssid;
String password;
String uid;
String api;

// HTML form with enhanced UI and character length constraints
const char* htmlForm = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP32 Setup</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background-color: #f4f4f9;
      margin: 0;
      padding: 0;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
    }
    .container {
      background-color: #fff;
      padding: 20px;
      box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
      border-radius: 10px;
      width: 300px;
      text-align: center;
    }
    h1 {
      color: #333;
      margin-bottom: 20px;
    }
    label {
      display: block;
      margin-bottom: 5px;
      color: #555;
      text-align: left;
    }
    input[type="text"], input[type="password"] {
      width: 100%;
      padding: 10px;
      margin: 10px 0;
      border: 1px solid #ccc;
      border-radius: 5px;
      box-sizing: border-box;
    }
    input[type="submit"] {
      background-color: #4CAF50;
      color: white;
      border: none;
      padding: 10px 20px;
      text-align: center;
      text-decoration: none;
      display: inline-block;
      font-size: 16px;
      margin-top: 20px;
      cursor: pointer;
      border-radius: 5px;
      width:100%;
    }
    input[type="submit"]:hover {
      background-color: #45a049;
    }
  </style>
  <script>
    function validateForm() {
      var ssid = document.getElementById("ssid").value;
      var password = document.getElementById("password").value;
      var uid = document.getElementById("uid").value;
      var api = document.getElementById("api").value;

      if (ssid.length < 5 || password.length < 5 || uid.length < 5 || uid.length > 16 || api.length < 5) {
        alert("All fields must contain at least 5 characters. UID must be between 5 and 16 characters.");
        return false;
      }
      return true;
    }
  </script>
</head>
<body>
  <div class="container">
    <h1>ESP32 Setup</h1>
    <form action="/save" method="POST" onsubmit="return validateForm()">
      <label for="ssid">SSID:</label>
      <input type="text" id="ssid" name="ssid" minlength="5" required><br>
      <label for="password">Password:</label>
      <input type="password" id="password" name="password" minlength="5" required><br>
      <label for="uid">UID:</label>
      <input type="text" id="uid" name="uid" minlength="5" maxlength="16" required><br>
      <label for="api">API Key:</label>
      <input type="text" id="api" name="api" minlength="5" required><br>
      <input type="submit" value="Save">
    </form>
  </div>
</body>
</html>)rawliteral";

// Handle the root URL ("/")
void handleRoot() {
  server.send(200, "text/html", htmlForm);
}

// Handle the form submission and save data
void handleSave() {
  ssid = server.arg("ssid");
  password = server.arg("password");
  uid = server.arg("uid");
  api = server.arg("api");

  // Save the data to non-volatile storage
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.putString("uid", uid);
  preferences.putString("api", api);

  // Create a response message
  String response = R"rawliteral(
  <!DOCTYPE HTML><html>
  <head>
    <title>ESP32 Setup</title>
    <style>
      body {
        font-family: Arial, sans-serif;
        background-color: #f4f4f9;
        margin: 0;
        padding: 0;
        display: flex;
        justify-content: center;
        align-items: center;
        height: 100vh;
      }
      .container {
        background-color: #fff;
        padding: 20px;
        box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
        border-radius: 10px;
        width: 300px;
        text-align: center;
      }
      h1 {
        color: #333;
        margin-bottom: 20px;
      }
      p {
        color: #555;
        text-align: left;
      }
    </style>
  </head>
  <body>
    <div class="container">
      <h1>Settings Saved</h1>
      <p>SSID: )rawliteral"
                    + ssid + R"rawliteral(</p>
      <p>Password: )rawliteral"
                    + password + R"rawliteral(</p>
      <p>UID: )rawliteral"
                    + uid + R"rawliteral(</p>
      <p>API Key: )rawliteral"
                    + api + R"rawliteral(</p>
      <p>Your Response have been Sucessfully Saved!</p>
      <p>You May Disconnect from ESP32-Setup</p>
    </div>
  </body>
  </html>)rawliteral";

  // Send the response back to the client
  server.send(200, "text/html", response);

  // Print the saved data to the serial monitor for debugging
#ifndef DEBUG

  Serial.println("Settings saved:");
  Serial.println("SSID: " + ssid);
  Serial.println("Password: " + password);
  Serial.println("UID: " + uid);
  Serial.println("API: " + api);
#endif
  // Connect to the provided Wi-Fi network
  WiFi.begin(ssid.c_str(), password.c_str());

  // Wait for connection
#ifndef DEBUG
  Serial.println("Connecting to WiFi..");
#endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
#ifndef DEBUG
    Serial.print(".");
#endif
  }

// Print the IP address of the ESP32 once connected to Wi-Fi
#ifndef DEBUG
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
#endif
}


void setup_wifi_credentials() {
  // Start preferences storage
  preferences.begin("esp32", false);
  // Get the saved ssid and password
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  uid = preferences.getString("uid", "");
  api = preferences.getString("api", "");
  if (ssid.length() < 5 || password.length() < 5) {
    // Set up the ESP32 as an access point
    WiFi.softAP("ESP32-Setup For BECA -- RF");
    // Start the web server
    server.on("/", HTTP_GET, handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.begin();
#ifndef DEBUG
    Serial.println("Web server started");

    // Print the IP address of the ESP32 access point
    Serial.print("Connect to Wi-Fi network 'ESP32-Setup' and navigate to: ");
    Serial.println(WiFi.softAPIP());
#endif
  } else {
    // Connect to the provided Wi-Fi network
    WiFi.begin(ssid.c_str(), password.c_str());
#ifndef DEBUG
    Serial.println("Previously saved settings:");
    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);
    Serial.println("UID: " + uid);
    Serial.println("API: " + api);
    Serial.println("Connecting to WiFi..");
#endif
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
#ifndef DEBUG
      Serial.print(".");
#endif
    }

// Print the IP address of the ESP32 once connected to Wi-Fi
#ifndef DEBUG
    Serial.println();
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
#endif
  }
}

void handleDelete() {
  // Delete stored credentials from preferences


  // Add Status Flag


  preferences.remove("ssid");
  preferences.remove("password");
  preferences.remove("uid");
  preferences.remove("api");

  // Restart ESP32
  ESP.restart();
}
