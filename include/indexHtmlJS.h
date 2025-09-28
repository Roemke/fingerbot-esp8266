#include <pgmspace.h>

const char index_html[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang='de'>
<head>
<meta charset='utf-8'>
<meta name='viewport' content='width=device-width'>
<title>Rollo Steuerung</title>
<style>
  body { font-family: Arial, sans-serif; margin: 0; padding: 0; }
  .tab { overflow: hidden; background-color: #333; }
  .tab button {
    background-color: inherit; float: left; border: none; outline: none;
    cursor: pointer; padding: 14px 16px; color: white;
  }
  .tab button:hover { background-color: #555; }
  .tab button.active { background-color: #111; }
  .tabcontent { display: none; padding: 20px; }
  .btn { padding: 10px 20px; margin: 10px; font-size: 16px; }
  .slider-container { margin: 15px 0; }
  .slider-container label { display: block; margin-bottom: 5px; }
  input[type=range], input[type=number] { width: 100%; }
  .btn.active {
    background-color: green;
    color: white;
  }
  .invisible {
    display: none;
  }
  .infoField {
    background-color: #008000; 
    padding: 10px; 
    border: 10px inset #004000;
    margin-top: 10px;
  }  

</style>

<script>
let gateway = `ws://${window.location.hostname}/ws`;
let websocket;

function setRolloState(state) 
{
    // erst alle Buttons deaktivieren
    ["btnUp", "btnDown", "btnStop"].forEach(id => {
        document.getElementById(id).classList.remove("active");
    });

    // dann den passenden Button aktivieren
    if (state === "up") {
        document.getElementById("btnUp").classList.add("active");
    } 
    else if (state === "down") {
        document.getElementById("btnDown").classList.add("active");
    }
    else if (state === "stop" || state === "middle") {
        document.getElementById("btnStop").classList.add("active");
    }
}

function initWebSocket() {
    websocket = new WebSocket(gateway);
    websocket.onopen = () => console.log("WebSocket verbunden");
    websocket.onclose = () => setTimeout(initWebSocket, 2000);

    websocket.onmessage = (e) => {
        console.log("Nachricht:", e.data);
        let data = JSON.parse(e.data);
        if (data.action !== "rolloState") 
        {  
          const el = document.getElementById(data.action);//slider setzen
          const elVal = document.getElementById(data.action+"Val");//wert setzen
          if (elVal) //nur wenn es das Element gibt
            elVal.innerText = data.value;
          if (el && el.value != data.value) //nur bei änderung setzen        
            el.value = data.value;
          }
        else
          setRolloState(data.value);

    };
}

function actualizeNumerics(action, value) 
{
    //zahlenwerte sofort setzen
    const elVal = document.getElementById(action+"Val");//wert setzen
    if (elVal) //nur wenn es das Element gibt
      elVal.innerText = value;
}

function sendAction(action, value="") 
{
    if (websocket && websocket.readyState === WebSocket.OPEN)
        websocket.send(JSON.stringify({ action: action, value: value }));    
}

//noch nicht angeschaut, von chatgpt generiert
function openTab(evt, tabName) {
    let i, tabcontent, tablinks;
    tabcontent = document.getElementsByClassName("tabcontent");
    for (i = 0; i < tabcontent.length; i++) { tabcontent[i].style.display = "none"; }
    tablinks = document.getElementsByClassName("tablink");
    for (i = 0; i < tablinks.length; i++) { tablinks[i].className = tablinks[i].className.replace(" active", ""); }
    document.getElementById(tabName).style.display = "block";
    evt.currentTarget.className += " active";
}

window.addEventListener("load", () => {
    initWebSocket();
    document.getElementById("defaultTab").click();
    document.getElementById("btnUp").addEventListener("click", () => {
        sendAction("rollo", "up");
    });
    document.getElementById("btnDown").addEventListener("click", () => {
        sendAction("rollo", "down");
    });
    document.getElementById("btnStop").addEventListener("click", () => {
        sendAction("rollo", "stop");
    });
    document.getElementById("btnUp").addEventListener("click", () => sendAction("rollo", "up"));
    document.getElementById("btnDown").addEventListener("click", () => sendAction("rollo", "down"));

    document.getElementById("servoLeft").addEventListener("change", e => sendAction("servoLeft", e.target.value));
    document.getElementById("servoMiddle").addEventListener("change", e => sendAction("servoMiddle", e.target.value));
    document.getElementById("servoRight").addEventListener("change", e => sendAction("servoRight", e.target.value));
    document.getElementById("servoLeft").addEventListener("input", e => actualizeNumerics("servoLeft", e.target.value));
    document.getElementById("servoMiddle").addEventListener("input", e => actualizeNumerics("servoMiddle", e.target.value));
    document.getElementById("servoRight").addEventListener("input", e => actualizeNumerics("servoRight", e.target.value));
    document.getElementById("servoPin").addEventListener("input", e => sendAction("servoPin", e.target.value));
    document.getElementById("timeDown").addEventListener("change", e => sendAction("timeDown", e.target.value));
    document.getElementById("timeUp").addEventListener("change", e => sendAction("timeUp", e.target.value)); 
    document.getElementById("timeDown").addEventListener("input", e => actualizeNumerics("timeDown", e.target.value));
    document.getElementById("timeUp").addEventListener("input", e => actualizeNumerics("timeUp", e.target.value)); 

    //fuer wlan setup
    document.getElementById("btnWifiSave").addEventListener("click", () => {
        const ssid = document.getElementById("wifiSsid").value;
        const pass = document.getElementById("wifiPass").value;
        sendAction("wifiSetCredentials", { ssid: ssid, password: pass });
        document.getElementById("wifiInfo").classList.remove("invisible");
    });
});
</script>
</head>

<body>
<div class="tab">
  <button class="tablink" id="defaultTab" onclick="openTab(event, 'steuerung')">Steuerung</button>
  <button class="tablink" onclick="openTab(event, 'setup')">Setup</button>
  <button class="tablink" onclick="openTab(event, 'wifi')">WLAN</button>
</div>

<div id="steuerung" class="tabcontent">
  <h2>Rollo Steuerung</h2>
  <button class="btn" id="btnUp">Hoch</button>
  <button class="btn active" id="btnStop">Stop</button>
  <button class="btn" id="btnDown">Runter</button>
</div>

<div id="setup" class="tabcontent">
  <h2>Servo Setup</h2>
  <div class="slider-container">
    <label for="servoLeft">Linke Endposition <span id="servoLeftVal">%SERVO_LEFT%</span></label>
    <input type="range" min="0" max="180" value="%SERVO_LEFT%" id="servoLeft">
  </div>
  <div class="slider-container">
    <label for="servoMiddle">Mittelposition <span id="servoMiddleVal">%SERVO_MIDDLE%</span></label>
    <input type="range" min="0" max="180" value="%SERVO_MIDDLE%" id="servoMiddle">
  </div>
  <div class="slider-container">
    <label for="servoRight">Rechte Endposition <span id="servoRightVal">%SERVO_RIGHT%</span></label>
    <input type="range" min="0" max="180" value="%SERVO_RIGHT%" id="servoRight">
  </div>
  <div class="slider-container">
    <label for="timeDown">Zeit Rollo runter (ms) <span id="timeDownVal">%TIME_DOWN%</span></label>
    <input type="range" min="100" max="10000" value="%TIME_DOWN%" id="timeDown">
  </div>
  <div class="slider-container">
    <label for="timeUp">Zeit Rollo hoch (ms) <span id="timeUpVal">%TIME_UP%</span></label>
    <input type="range" min="100" max="10000" value="%TIME_UP%" id="timeUp">
  </div>
  <div class="slider-container">
    <label for="servoPin">Servo Pin</label>
    <input type="number" min="0" max="39" value="%SERVO_PIN%" id="servoPin">
  </div>
</div>
<div id="wifi" class="tabcontent">
  <h2>WLAN Konfiguration</h2>
  <p>Mac-Adresse im AP-Modus: %WIFI_MAC_AP%</p>
  <p>Mac-Adresse im Client-Modus (STA): %WIFI_MAC_STA%</p>
  <p>aktueller Modus: %WIFI_MAC_MODE%</p>
  <div class="slider-container">
    <label for="wifiSsid">SSID</label>
    <input type="text" id="wifiSsid" placeholder="Netzwerkname">
  </div>
  <div class="slider-container">
    <label for="wifiPass">Passwort</label>
    <input type="password" id="wifiPass" placeholder="Passwort">
  </div>
  <button class="btn" id="btnWifiSave">Speichern</button>
  <div class="infoField invisible" id="wifiInfo">
    <strong>Daten übermittelt.</strong><br>  
    <strong>Hinweis:</strong> Nach dem Speichern der WLAN-Daten muss der ESP neu gestartet werden, 
    in der Regel geschieht dies automatisch.  Bitte ansonsten den ESP manuell neu starten.  
  </div>
</div>

</body>
</html>
)rawliteral";
