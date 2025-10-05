#include <pgmspace.h>

const char index_html[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang='de'>
<head>
<meta charset='utf-8'>
<meta name='viewport' content='width=device-width'>
<title>Markisen Steuerung</title>
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

function setButtonState(state) 
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

function addLogLine(line) {
    const logContainer = document.getElementById("logContainer");
    if (!logContainer) return;

    const div = document.createElement("div");
    div.textContent = line;
    logContainer.appendChild(div);
    logContainer.scrollTop = logContainer.scrollHeight; // scrollt automatisch nach unten
}

function initWebSocket() {
    websocket = new WebSocket(gateway);
    websocket.onopen = () => console.log("WebSocket verbunden");
    websocket.onclose = () => setTimeout(initWebSocket, 2000);

    websocket.onmessage = (e) => {
        console.log("Nachricht:", e.data);
        let data = JSON.parse(e.data);
        if (data.action === "init") 
        {
          // Initialdaten vom ESP: alle Werte auf einmal
          // → in die UI eintragen
          setButtonState(data.buttonState);

          // Alle relevanten Slider/Felder setzen
          [
            "servoLeft","servoMiddle","servoRight",
            "servoStopActive","servoStopInactive",
            "timePress","servoPinUpDown","servoPinStop"
          ].forEach(id => {
              if (data[id] !== undefined) 
              {
                  const el = document.getElementById(id);
                  const elVal = document.getElementById(id + "Val");
                  if (el) el.value = data[id];
                  if (elVal) elVal.innerText = data[id];
              }
          });
          data.logs.forEach(line => addLogLine(line));
        }
        else if (data.action === "log") 
        {
          addLogLine(data.line);
        }
        else if (data.action !== "button") 
        {  
          const el = document.getElementById(data.action);//slider setzen
          const elVal = document.getElementById(data.action+"Val");//wert setzen
          if (elVal) //nur wenn es das Element gibt
            elVal.innerText = data.value;
          if (el && el.value != data.value) //nur bei änderung setzen        
            el.value = data.value;
          }
        else
          setButtonState(data.value);

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

function initUI() {
    initWebSocket();
    document.getElementById("defaultTab").click();

    // Buttons
    ["btnUp", "btnStop", "btnDown"].forEach(btnId => {
        const actionMap = { "btnUp":"up", "btnStop":"stop", "btnDown":"down" };
        const btn = document.getElementById(btnId);
        if (btn) btn.addEventListener("click", () => sendAction("button", actionMap[btnId]));
    });

    // Slider
    ["servoLeft","servoMiddle","servoRight","servoStopActive","servoStopInactive"].forEach(id => {
        const el = document.getElementById(id);
        if (!el) return;
        el.addEventListener("input", e =>  actualizeNumerics(id, e.target.value));        
        el.addEventListener("input", e => sendAction(id, e.target.value));
        
    });

    // Pins + Zeit
    ["servoPinUpDown","servoPinStop","timePress"].forEach(id => {
        const el = document.getElementById(id);
        if (!el) return;
        el.addEventListener("change", e => sendAction(id, e.target.value));
        if (id === "timePress") {
            el.addEventListener("input", e => actualizeNumerics(id, e.target.value));
        }
    });

    // WLAN speichern
    const btnWifi = document.getElementById("btnWifiSave");
    if (btnWifi) btnWifi.addEventListener("click", () => {
        sendAction("wifiSetCredentials", {
            ssid: document.getElementById("wifiSsid").value,
            password: document.getElementById("wifiPass").value
        });
        document.getElementById("wifiInfo").classList.remove("invisible");
    });
}
window.addEventListener("load", initUI);
</script>
</head>

<body>
<div class="tab">
  <button class="tablink" id="defaultTab" onclick="openTab(event, 'steuerung')">Steuerung</button>
  <button class="tablink" onclick="openTab(event, 'help')">Help</button>
  <button class="tablink" onclick="openTab(event, 'setup')">Setup</button>
  <button class="tablink" onclick="openTab(event, 'wifi')">WLAN</button>
</div>

<div id="steuerung" class="tabcontent">
  <h2>Markise Steuerung</h2>
  <button class="btn" id="btnUp">Markise rein</button>
  <button class="btn active" id="btnStop">Stop</button>
  <button class="btn" id="btnDown">Markise raus</button>
</div>
<div id="help" class="tabcontent">
  <h2>Help</h2>
  <p>Hier werden die Funktionen erklärt:</p>
  <ul>
    <li>Steuerung der Markise</li>
    <li>Setup der Servo-Endpunkte</li>
    <li>WLAN Konfiguration</li>
    <li>OTA-Update über <a href="/update">diesen Link</a></li>
  </ul>

  <!-- Logs ans Ende des Help-Tabs -->
  <h3>Log-Nachrichten</h3>
  <div id="logContainer" style="height:300px; overflow:auto; background:#f0f0f0; padding:10px; font-family:monospace;"></div>
</div>
<div id="setup" class="tabcontent">
  <h2>Servo Setup</h2>
  <div class="slider-container">
    <label for="servoLeft">Linke Endposition / down, Markise raus <span id="servoLeftVal">%SERVO_LEFT%</span></label>
    <input type="range" min="0" max="180" value="%SERVO_LEFT%" id="servoLeft">
  </div>
  <div class="slider-container">
    <label for="servoMiddle">Mittelposition <span id="servoMiddleVal">%SERVO_MIDDLE%</span></label>
    <input type="range" min="0" max="180" value="%SERVO_MIDDLE%" id="servoMiddle">
  </div>
  <div class="slider-container">
    <label for="servoRight">Rechte Endposition / up, Markise rein <span id="servoRightVal">%SERVO_RIGHT%</span></label>
    <input type="range" min="0" max="180" value="%SERVO_RIGHT%" id="servoRight">
  </div>
  <div class="slider-container">
    <label for="servoStopActive">Position Stop-Servo aktiv <span id="servoStopActiveVal">%SERVO_STOP_ACTIVE%</span></label>
    <input type="range" min="0" max="180" value="%SERVO_STOP_ACTIVE%" id="servoStopActive">
  </div>
  <div class="slider-container">
    <label for="servoStopInactive">Position Stop-Servo inaktiv <span id="servoStopInActiveVal">%SERVO_STOP_INACTIVE%</span></label>
    <input type="range" min="0" max="180" value="%SERVO_STOP_INACTIVE%" id="servoStopInactive">
  </div>
  <div class="slider-container">
    <label for="timePress">Zeit Buttondruck (ms) <span id="timePressVal">%TIME_PRESS%</span></label>
    <input type="range" min="100" max="5000" value="%TIME_PRESS%" id="timePress">
  </div>
  <div class="slider-container">
    <label for="servoPinUpDown">Servo Pin Up/Down</label>
    <input type="number" min="0" max="39" value="%SERVO_PIN_UPDOWN%" id="servoPinUpDown">
  </div>
  <div class="slider-container">
    <label for="servoPinStop">Servo Pin Stop</label>
    <input type="number" min="0" max="39" value="%SERVO_PIN_STOP%" id="servoPinStop">
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
