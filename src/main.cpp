#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <AsyncUDP.h>
#include <AsyncElegantOTA.h>
#include <SPIFFS.h>
#include <GParser.h>
#include <FastBot.h>

#define Keenetic //dpe //MKD_Guest 123
#ifdef MKD_Guest
const char *ssid = "MKD-Guest";
const char *password = "123Qweasd";
#endif
#ifdef Keenetic
const char *ssid = "Keenetic-1649";
const char *password = "jsCMnJpr";
#endif
#ifdef dpe
const char *ssid = "dpe";
const char *password = "11015385";
#endif


#define DEBUG
#define SOFTAP
// https://api.telegram.org/bot6514408612:AAHjaCKlpjAPjK9jXNL7CsVLWwlgp7QUF38/getUpdates
// https://api.telegram.org/bot6514408612:AAHjaCKlpjAPjK9jXNL7CsVLWwlgp7QUF38/sendMessage?chat_id=@[MKD Horizon]&text=тест
#define BOT_TOKEN "6514408612:AAHjaCKlpjAPjK9jXNL7CsVLWwlgp7QUF38"
#define DPE_CHAT_ID "213100274"
//#define CHAT_ID "-1001765861822"

FastBot bot(BOT_TOKEN);

const uint16_t port = 1234;
AsyncUDP udp;

char Bridge_buf[32] = "0,0,127,127,127;";
char Spreader_buf[32] = "1,0,127,127,0;";
char Mini_buf[32] = "2,0,0,0,127,127,127;";
char Lift_buf[32] = "3,0,90,6;";

uint8_t twistLock = 0;
uint8_t SyncMode = 0;
uint8_t ActiveCrane = 0;

uint8_t B_Online = 0;
uint8_t S_Online = 0;
uint8_t M_Online = 2;
uint8_t L_Online = 0;

uint32_t lastReceiveTime1 = millis();
uint32_t lastReceiveTime2 = millis();
uint32_t lastReceiveTime3 = millis();
uint32_t lastReceiveTime4 = millis();

String B_IP = "0.0.0.0";
String S_IP = "0.0.0.0";
String M_IP = "0.0.0.0";
String L_IP = "0.0.0.0";

void checkClientsOnline();
void statusHandler(AsyncWebServerRequest *request);
void commandHandler(AsyncWebServerRequest *request);
void indexHandler(AsyncWebServerRequest *request);
void onPacketEvent(AsyncUDPPacket packet);
void newMsg(FB_msg &msg);
String IpAddress2String(const IPAddress& ipAddress);
String StatusAnswer();
void preflight(AsyncWebServerRequest *request);


char status[4];

AsyncWebServer server(80);

//<i class="i_mask" style="-webkit-mask:center/contain no-repeat url(/Btu.svg);width:80px;height:80px;"></i>
#pragma region
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<html>
<head>
    <title>MKD HORIZONE</title>
    <style type="text/css">
        input[type="range"] {
            -webkit-appearance: none;
            appearance: none;
            background: lightblue;
            cursor: pointer;
            height: 10px;
            border-radius: 7px;
            margin: 30px;
            width: 400px;
        }

        input[type=range]::-webkit-slider-thumb {
            height: 20px;
            width: 10px;
            border-radius: 3px;
            background: #004C8F;
            cursor: pointer;
            -webkit-appearance: none;
            margin-top: -0px;
        }

        body {
            font-family: Arial;
            text-align: center;
            margin: 0px auto;
            padding-top: 30px;
        }

        table {
            margin-left: auto;
            margin-right: auto;
        }

        td {
            padding: 4 px;
            text-align: center;
        }

        .button {
            background-color: #2f4468;
            border: none;
            color: white;
            padding: 10px;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 18px;
            margin: 3px;
            cursor: pointer;
            -webkit-touch-callout: none;
            -webkit-user-select: none;
            -khtml-user-select: none;
            -moz-user-select: none;
            -ms-user-select: none;
            user-select: none;
            -webkit-tap-highlight-color: rgba(0, 0, 0, 0);
        }

        .menu_item {
            margin: 25px;
        }

        a {
            outline-color: transparent;
        }

        a:link {
            color: #ff9900c2;
            text-decoration: none;
            font-weight: bold;
            font-size: larger;
        }

        a:visited {
            color: #ff9900c2;
            text-decoration: none;
        }

        a:focus {
            text-decoration: none;
        }

        a:hover {
            text-decoration: none;
        }

        a:active {
            text-decoration: none;
        }

        .red {
            color: red;
        }
        .green {
            color: green;
        }
        .blue {
            color: blue;
        }
    </style>
</head>

<body>
    <h1>MKD HORIZONE</h1>
    <div>
        <a href="#" class="menu_item" onclick="toggleMenu(0);">BRIDGE<span id="bon" class="blue">*</span></a>
        <a href="#" class="menu_item" onclick="toggleMenu(1);">SPREADER<span id="son" class="blue">*</span></a>
        <a href="#" class="menu_item" onclick="toggleMenu(2);">MINI<span id="mon" class="blue">*</span></a>
        <a href="#" class="menu_item" onclick="toggleMenu(3);">LIFT<span id="lon" class="blue">*</span></a>
    </div>

    <div>
        <input type="range" id="rate" min="0" max="100" value="50" />
    </div>

    <div id="navbar0" style="display: block;">
        <table>
            <tr>
                <td colspan="2">
                    <input type="button" onmousedown="toggleButton('Tl');" ontouchstart="toggleButton('Tl');" 
                        value="Trolley Left" /></td>
                <td><input type="button" onmousedown="toggleButton('Wu');" ontouchstart="toggleButton('Wu');"
                        value="Winch Up" /></td>
            </tr>
            <tr>
                <td><input type="button" onmousedown="toggleButton('Bb');" ontouchstart="toggleButton('Bb');"
                        value="Bridge Backward" /></td>
                <td><input type="button" onmousedown="toggleButton('Bf');" ontouchstart="toggleButton('Bf');"
                        value="Bridge Forward" /></td>
                <td>&nbsp;</td>
            </tr>
            <tr>
                <td colspan="2">
                    <input type="button" onmousedown="toggleButton('Tr');" ontouchstart="toggleButton('Tr');" 
                        value="Trolley Right" /></td>
                <td><input type="button" onmousedown="toggleButton('Wd');" ontouchstart="toggleButton('Wd');"
                        value="Winch Down" /></td>
            </tr>
        </table>
    </div>
    <div id="navbar1" style="display: none;">
        <table>
            <tr>
                <td><input type="button" onmousedown="toggleButton('Ex');" ontouchstart="toggleButton('Ex');"
                        value="Reduce telescopic" /></td>
                <td><input type="button" onmousedown="toggleButton('Rc');" ontouchstart="toggleButton('Rc');"
                        value="Turn clockwise" /></td>
                <td><input type="button" onmousedown="toggleButton('Xl');" ontouchstart="toggleButton('Xl');"
                        value="Lock twistlock" /></td>
            </tr>
            <tr>
                <td><input type="button" onmousedown="toggleButton('En');" ontouchstart="toggleButton('En');"
                        value="Increase telescopic" /></td>
                <td><input type="button" onmousedown="toggleButton('Rw');" ontouchstart="toggleButton('Rw');"
                        value="Turn counter-clockwise" /></td>
                <td><input type="button" onmousedown="toggleButton('Xu');" ontouchstart="toggleButton('Xu');"
                        value="Unlock twistlock" /></td>
            </tr>
        </table>
    </div>
    <div id="navbar2" style="display: none;">
        <table>
            <tr>
                <td><input type="radio" name="SY" id="M0" onmousedown="toggleCheckbox('M0');" ontouchstart="toggleCheckbox('M0');"
                        value="Sync Mode" />Twin Mode</td>
                <td><input type="button" onmousedown="toggleButton('Mc');" ontouchstart="toggleButton('Mc');"
                        value="Turn CC" /></td>
                <td><input type="button" onmousedown="toggleButton('Mw');" ontouchstart="toggleButton('Mw');"
                        value="Turn CCW" /></td>
            </tr>
            <tr>
                <td><input type="radio" name="SY" id="M1" onmousedown="toggleCheckbox('M1');" ontouchstart="toggleCheckbox('M1');"
                        value="Crane #1" />Crane #1</td>
                <td><input type="button" onmousedown="toggleButton('Mu');" ontouchstart="toggleButton('Mu');"
                        value="Arm Up" /></td>
                <td><input type="button" onmousedown="toggleButton('Md');" ontouchstart="toggleButton('Md');"
                        value="Arm Down" /></td>
            </tr>
            <tr>
                <td><input type="radio" name="SY" id="M2" onmousedown="toggleCheckbox('M2');" ontouchstart="toggleCheckbox('M2');"
                        value="Crane #2" />Crane #2</td>
                <td><input type="button" onmousedown="toggleButton('Ml');" ontouchstart="toggleButton('Ml');"
                        value="Winch Up" /></td>
                <td><input type="button" onmousedown="toggleButton('Mn');" ontouchstart="toggleButton('Mn');"
                        value="Winch Down" /></td>
            </tr>
        </table>
    </div>
    <div id="navbar3" style="display: none;">
        <table>
            <tr>
                <td><input type="button" onmousedown="toggleButton('Lu');" ontouchstart="toggleButton('Lu');"
                        value="Lift Horizone UP" /></td>
            </tr>
            <tr>
                <td><input type="button" onmousedown="toggleButton('Ld');" ontouchstart="toggleButton('Ld');"
                        value="Dip Horizone DOWN" /></td>
            </tr>
        </table>
    </div>
    <div>
        <input type="button" value="Stop" />
    </div>

    <script type="text/javascript">
        var arr = "0000";
        window.addEventListener("mouseup", stopEvent);
        window.addEventListener("touchend", stopEvent);
        window.addEventListener("touchcancel", stopEvent);
        var intervalID = window.setInterval(requestStatus, 1500);

        var buttons = document.querySelectorAll('input[type=button]');
        for (var i = 0; i < buttons.length; i++) {
            buttons[i].classList.add('button');
        }

        function stopEvent() {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/action?go=stop&value=0", true);
            xhr.send();
        }

        function toggleButton(x) {
            var rate = document.getElementById('rate').value;
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/action?go=" + x + "&value=" + rate, true);
            xhr.onload = function() {
                if(xhr.status == 200 && xhr.readyState == 4){
                    //setStatus(xhr.responseText);
                }
            }
            xhr.send();
        }

        function toggleCheckbox(x) {
            var name = document.getElementById(x).id;
            var rate = 0;
            if(name == 'M0')
                rate = '2';
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/action?go=" + x + "&value=" + rate, true);
            xhr.onload = function() {
                if(xhr.status == 200 && xhr.readyState == 4){
                    //setStatus(xhr.responseText);
                }
            }
            xhr.send();
        }

        function toggleMenu(i) {
            for (let j = 0; j < 4; j++) {
                if (i != j)
                    document.getElementById("navbar" + j).style.display = "none";
            }
            document.getElementById("navbar" + i).style.display = "block";
        }

        function setStatus(str){
            arr = Array.from(str);
            console.log(arr);
            var bon = document.getElementById('bon');
            var son = document.getElementById('son');
            var mon = document.getElementById('mon');
            var lon = document.getElementById('lon');
            if(arr[0] == '1') bon.className = "green"; else bon.className = "red";
            if(arr[1] == '1') son.className = "green"; else son.className = "red";
            if(arr[2] == '1') mon.className = "green"; else mon.className = "red";
            if(arr[3] == '1') lon.className = "green"; else lon.className = "red";
        }

        function requestStatus(){
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/status", true);
            xhr.onload = function() {
                if(xhr.status == 200 && xhr.readyState == 4){
                    setStatus(xhr.responseText);
                }
            }
            xhr.send(); 
        }
    </script>
</body>
</html>
)rawliteral";
#pragma endregion

void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200);
  /*
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  */

WiFi.setHostname("HORIZONE");
#ifdef AP
  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAP(ssid, password);
#else
  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
#endif
  Serial.println("");

  server.on("/", HTTP_GET, indexHandler);
  server.on("/status", HTTP_GET, statusHandler);
  server.on("/action", HTTP_GET, commandHandler);
  server.on("/status", HTTP_OPTIONS, preflight);

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

  AsyncElegantOTA.begin(&server); // Start ElegantOTA

  // Ожидаем соединения

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);

  server.begin();
  Serial.print("Server Ready! Go to: http://");
#ifdef AP
  Serial.println(WiFi.softAPIP());
#else
  Serial.println(WiFi.localIP());
#endif

  if (udp.listen(port))
  {
    Serial.print("UDP Listening on IP: ");
#ifdef AP
    Serial.println(WiFi.softAPIP());
#else
    Serial.println(WiFi.localIP());
#endif
    udp.onPacket(onPacketEvent);
  }

  //bot.setChatID("");
  bot.attach(newMsg);
  //bot.showMenu("STATUS");
  bot.sendMessage(StatusAnswer(), DPE_CHAT_ID);
}

void loop()
{
  bot.tick();
  GParser data(Lift_buf); // Парсим и отправляем по UART только мощность двигателя (значение из второй ячейки)
  int ints[data.amount()];
  data.parseInts(ints);
  Serial2.write(ints[2]);

#if AP
#else
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
#endif
  AsyncElegantOTA.loop();
  checkClientsOnline();
  static uint32_t tmr = millis();
  if (millis() - tmr > 1000)
  {
    tmr = millis();
// char AP_ident[16];
// snprintf(AP_ident, sizeof(AP_ident), "$%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
#ifdef AP
    udp.broadcast(WiFi.softAPIP().toString().c_str());
    Serial.println(WiFi.softAPIP().toString().c_str());
#else
    udp.broadcast(WiFi.localIP().toString().c_str());
    Serial.println(WiFi.localIP().toString().c_str());
#endif
  }
}

void commandHandler(AsyncWebServerRequest *request)
{
  int paramsNr = request->params();
  Serial.println(paramsNr);

  String go = request->getParam("go")->value();
  String value = request->getParam("value")->value();

  ///
  ///
  int rate = value.toInt();
#ifdef DEBUG
  Serial.print("go[String]: ");
  Serial.println(go);
  Serial.print("rate[int]: ");
  Serial.println(rate);
#endif

  int a = rate * 128 / 100 + 127;
  int b = (100 - rate) * 127 / 100;

  int aa = rate * 90 / 100 + 90;
  int ab = (100 - rate) * 90 / 100;

  snprintf(Bridge_buf, sizeof(Bridge_buf), "0,0,127,127,127;");
  snprintf(Spreader_buf, sizeof(Spreader_buf), "1,0,127,127,%d;", twistLock);
  snprintf(Mini_buf, sizeof(Mini_buf), "2,0,%d,%d,127,127,127;", SyncMode, ActiveCrane);
  snprintf(Lift_buf, sizeof(Lift_buf), "3,0,90,6;");

  switch (go[0])
  {
  case 'B': // Bridge
  case 'T': // Trolley
  case 'W': // Winch
#ifdef DEBUG
    Serial.println("Bridge working");
#endif
    switch (go[1])
    {
    case 'f':
      snprintf(Bridge_buf, sizeof(Bridge_buf), "0,1,%d,127,127;", a);
      break;
    case 'b':
      snprintf(Bridge_buf, sizeof(Bridge_buf), "0,1,%d,127,127;", b);
      break;
    case 'r':
      snprintf(Bridge_buf, sizeof(Bridge_buf), "0,1,127,%d,127;", b);
      break;
    case 'l':
      snprintf(Bridge_buf, sizeof(Bridge_buf), "0,1,127,%d,127;", a);
      break;
    case 'u':
      snprintf(Bridge_buf, sizeof(Bridge_buf), "0,1,127,127,%d;", a);
      break;
    case 'd':
      snprintf(Bridge_buf, sizeof(Bridge_buf), "0,1,127,127,%d;", b);
      break;
    }
    break;

  case 'R': // Spreader Rotate
  case 'E': // Spreader Telescopes
  case 'X': // Spreader Twistlocks
#ifdef DEBUG
    Serial.println("Spreader working");
#endif
    switch (go[1])
    {
    case 'c':
      snprintf(Spreader_buf, sizeof(Spreader_buf), "1,1,%d,127,%d;", a, twistLock);
      break;
    case 'w':
      snprintf(Spreader_buf, sizeof(Spreader_buf), "1,1,%d,127,%d;", b, twistLock);
      break;
    case 'x':
      snprintf(Spreader_buf, sizeof(Spreader_buf), "1,1,127,%d,%d;", a, twistLock);
      break;
    case 'n':
      snprintf(Spreader_buf, sizeof(Spreader_buf), "1,1,127,%d,%d;", b, twistLock);
      break;
    case 'l':
      twistLock = 0;
      snprintf(Spreader_buf, sizeof(Spreader_buf), "1,1,127,127,%d;", twistLock);
      break;
    case 'u':
      twistLock = 180;
      snprintf(Spreader_buf, sizeof(Spreader_buf), "1,1,127,127,%d;", twistLock);
      break;
    }
    break;

  case 'L': // Lifts
#ifdef DEBUG
    Serial.println("Lift working");
#endif
    if (go[1] == 'u')
    {
      snprintf(Lift_buf, sizeof(Lift_buf), "3,1,%d,6;", aa);
    }
    else if (go[1] == 'd')
    {
      snprintf(Lift_buf, sizeof(Lift_buf), "3,1,%d,6;", ab);
    }
    break;

  case 'M':
#ifdef DEBUG
    Serial.println("Cranes working");
#endif
    switch (go[1])
    {
    case '2':
      SyncMode = rate;
      snprintf(Mini_buf, sizeof(Mini_buf), "2,1,%d,%d,127,127,127;", SyncMode, ActiveCrane);
      break;
    case '0':
      ActiveCrane = 0;
      snprintf(Mini_buf, sizeof(Mini_buf), "2,1,%d,%d,127,127,127;", SyncMode, ActiveCrane);
      break;
    case '1':
      ActiveCrane = 1;
      snprintf(Mini_buf, sizeof(Mini_buf), "2,1,%d,%d,127,127,127;", SyncMode, ActiveCrane);
      break;
    case 'c':
      snprintf(Mini_buf, sizeof(Mini_buf), "2,1,%d,%d,%d,127,127;", SyncMode, ActiveCrane, a);
      break;
    case 'w':
      snprintf(Mini_buf, sizeof(Mini_buf), "2,1,%d,%d,%d,127,127;", SyncMode, ActiveCrane, b);
      break;
    case 'u':
      snprintf(Mini_buf, sizeof(Mini_buf), "2,1,%d,%d,127,%d,127;", SyncMode, ActiveCrane, a);
      break;
    case 'd':
      snprintf(Mini_buf, sizeof(Mini_buf), "2,1,%d,%d,127,%d,127;", SyncMode, ActiveCrane, b);
      break;
    case 'l':
      snprintf(Mini_buf, sizeof(Mini_buf), "2,1,%d,%d,127,127,%d;", SyncMode, ActiveCrane, a);
      break;
    case 'n':
      snprintf(Mini_buf, sizeof(Mini_buf), "2,1,%d,%d,127,127,%d;", SyncMode, ActiveCrane, b);
      break;
    }
    break;
  }
///
#ifdef DEBUG
  Serial.println(Bridge_buf);
  Serial.println(Spreader_buf);
  Serial.println(Mini_buf);
  Serial.println(Lift_buf);
#endif
  ///
  sprintf(status, "%d%d%d%d", B_Online, S_Online, M_Online, L_Online);
  request->send(200, "text/plain", status);
}

void indexHandler(AsyncWebServerRequest *request)
{
  request->send(200, "text/html", INDEX_HTML);
  // request->send(SPIFFS, "/index.html", "text/html");
  // request->send(SPIFFS, "/index.html", String(), false);
}

void statusHandler(AsyncWebServerRequest *request)
{
  sprintf(status, "%d%d%d%d", B_Online, S_Online, M_Online, L_Online);
  request->send(200, "text/plain", status);
}

void onPacketEvent(AsyncUDPPacket packet)
{
#ifdef DEBUG
  Serial.print("UDP Packet Type: ");
  Serial.print(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast"
                                                                         : "Unicast");
  Serial.print(", From: ");
  Serial.print(packet.remoteIP());
  Serial.print(":");
  Serial.print(packet.remotePort());
  Serial.print(", To: ");
  Serial.print(packet.localIP());
  Serial.print(":");
  Serial.print(packet.localPort());
  Serial.print(", Length: ");
  Serial.print(packet.length());
  Serial.print(", Data: ");
  // Serial.write(packet.data(), packet.length());
  // packet.printf("Got %u bytes of data", packet.length());
  Serial.println();
#endif

  // reply to the client
  char buf[32] = "";
  strncat(buf, (char *)packet.data(), packet.length());
  if (buf[0] == '_')
  {
    int send = 0;
    switch (buf[1])
    {
    case 'B':
      B_Online = 1;
      B_IP = IpAddress2String(packet.remoteIP());
      lastReceiveTime1 = millis();
      packet.printf(Bridge_buf, sizeof(Bridge_buf));
      break;
    case 'S':
      S_Online = 1;
      S_IP = IpAddress2String(packet.remoteIP());
      lastReceiveTime2 = millis();
      packet.printf(Spreader_buf, sizeof(Spreader_buf));
      break;
    case 'M':
      M_Online = 1;
      M_IP = IpAddress2String(packet.remoteIP());
      lastReceiveTime3 = millis();
      packet.printf(Mini_buf, sizeof(Mini_buf));
      break;
    case 'L':
      L_Online = 1;
      L_IP = IpAddress2String(packet.remoteIP());
      lastReceiveTime4 = millis();
      packet.printf(Lift_buf, sizeof(Lift_buf));
      break;
    }
  }
}

void checkClientsOnline()
{
  uint16_t interv = 1500;
  if (millis() - lastReceiveTime1 > interv)
    B_Online = 0;
  if (millis() - lastReceiveTime2 > interv)
    S_Online = 0;
  if (millis() - lastReceiveTime3 > interv)
    M_Online = 2;
  if (millis() - lastReceiveTime4 > interv)
    L_Online = 0;
}

void preflight(AsyncWebServerRequest *request)
{
  request->send(200, "text/plain", "kek");
}

void newMsg(FB_msg &msg)
{
  Serial.println("Message from BOT");
  Serial.println(msg.toString());
  //bot.setChatID(msg.chatID);
  //Serial.println(msg.chatID);
  if (msg.text == "/status")
  {    
    bot.sendMessage(StatusAnswer(), msg.chatID);
    //bot.setChatID("");
  }

  if (msg.OTA) bot.update();
}

String IpAddress2String(const IPAddress& ipAddress)
{
    return String(ipAddress[0]) + String(".") +
           String(ipAddress[1]) + String(".") +
           String(ipAddress[2]) + String(".") +
           String(ipAddress[3]);
}

String StatusAnswer()
{
    String answer = "";
    answer += "Connected to SSID:\t" + (String)ssid + "\n";
    answer += "SSID password:\t" + (String)password + "\n\n";
    answer += "🟢  Horizone base\thttp://" + IpAddress2String(WiFi.localIP()) + "\n";
    answer += "🟢  Horizone base update\thttp://" + IpAddress2String(WiFi.localIP()) + "/update\n";
    answer += B_Online == 1 ? "🟢" : "🔴";
    answer += "  Bridge ";
    answer += B_Online == 1 ? ("http://" + B_IP + "/update\n") : "\n";

    answer += S_Online == 1 ? "🟢" : "🔴";
    answer += "  Spreader ";
    answer += S_Online == 1 ? ("http://" + S_IP + "/update\n") : "\n";

    //answer += M_Online == 1 ? "🟢" : "🔴";
    //answer += "  Cranes ";
    //answer += M_Online == 1 ? ("http://" + M_IP + "/update\n") : "\n";

    //answer += L_Online == 1 ? "🟢" : "🔴";
    //answer += "  Lift ";
    //answer += L_Online == 1 ? ("http://" + L_IP + "/update\n") : "\n";
    
    return(answer);
}