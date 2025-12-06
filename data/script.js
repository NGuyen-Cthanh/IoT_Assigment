// ==================== GLOBAL VARIABLES ====================
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var gaugeTemp, gaugeHumi; 

// ==================== WEBSOCKET ====================
window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
    initGauges();
    // Load relays from local storage if you want persistence, otherwise starts empty
    // renderRelays(); 
}

function initWebSocket() {
    console.log('Connecting to WebSocket...');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connected');
    document.getElementById('connectionStatus').classList.add('connected');
    document.getElementById('statusText').innerText = "Connected";
}

function onClose(event) {
    console.log('Disconnected');
    document.getElementById('connectionStatus').classList.remove('connected');
    document.getElementById('statusText').innerText = "Disconnected";
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    try {
        var data = JSON.parse(event.data);

        // 1. UPDATE GAUGES
        if (data.hasOwnProperty('temperature')) {
            gaugeTemp.refresh(data.temperature);
        }
        if (data.hasOwnProperty('humidity')) {
            gaugeHumi.refresh(data.humidity);
        }

        // 2. UPDATE DEVICE STATUS FROM SERVER (Optional Sync)
        // If the server sends back status updates, handle them here.

    } catch (e) {
        console.warn("Invalid JSON:", event.data);
    }
}

function Send_Data(data) {
    if (websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send(data);
    }
}

// ==================== UI LOGIC ====================
let relayList = [];
let deleteTarget = null;

function showSection(id, event) {
    document.querySelectorAll('.view-section').forEach(sec => sec.style.display = 'none');
    document.getElementById(id).style.display = 'block';
    document.querySelectorAll('.nav-item').forEach(i => i.classList.remove('active'));
    event.currentTarget.classList.add('active');
}

function initGauges() {
    // Styled to match the CSS theme
    var commonOpts = {
        value: 0,
        min: 0,
        max: 100,
        donut: true,
        gaugeWidthScale: 0.4,
        counter: true,
        hideInnerShadow: true,
        gaugeColor: "#e5e7eb",
        levelColors: ["#6366f1"] // Matches CSS --accent
    };

    gaugeTemp = new JustGage({
        id: "gauge_temp",
        ...commonOpts,
        max: 50,
        label: "°C",
        levelColors: ["#f59e0b", "#ef4444"] // Orange to Red for temp
    });

    gaugeHumi = new JustGage({
        id: "gauge_humi",
        ...commonOpts,
        label: "%",
        levelColors: ["#3b82f6"] // Blue for humidity
    });
}

// ==================== RELAY LOGIC ====================
function openAddRelayDialog() {
    document.getElementById('addRelayDialog').style.display = 'flex';
}
function closeAddRelayDialog() {
    document.getElementById('addRelayDialog').style.display = 'none';
}

function saveRelay() {
    const name = document.getElementById('relayName').value.trim();
    const gpio = document.getElementById('relayGPIO').value.trim();
    if (!name || !gpio) return alert("Please fill all fields");
    
    relayList.push({ id: Date.now(), name, gpio, state: false });
    renderRelays();
    closeAddRelayDialog();
    
    // Clear inputs
    document.getElementById('relayName').value = '';
    document.getElementById('relayGPIO').value = '';
}

function renderRelays() {
    const container = document.getElementById('relayContainer');
    container.innerHTML = "";
    
    relayList.forEach(r => {
        const tile = document.createElement('div');
        // Add 'active' class if state is true for glowing effect
        tile.className = `device-tile ${r.state ? 'active' : ''}`;
        
        // The click event hits the whole tile
        tile.onclick = (e) => {
            // Prevent triggering toggle if clicking delete icon
            if(e.target.classList.contains('fa-trash') || e.target.classList.contains('tile-delete')) return;
            toggleRelay(r.id);
        };

        tile.innerHTML = `
            <div class="tile-header">
                <div class="icon-wrapper">
                    <i class="fa-solid fa-power-off"></i>
                </div>
                <div class="tile-delete" onclick="deleteRelay(${r.id})">
                    <i class="fa-solid fa-trash"></i>
                </div>
            </div>
            <div class="tile-info">
                <h3>${r.name}</h3>
                <p class="status">${r.state ? 'ON' : 'OFF'} • GPIO ${r.gpio}</p>
            </div>
        `;
        container.appendChild(tile);
    });
}

function toggleRelay(id) {
    const relay = relayList.find(r => r.id === id);
    if (relay) {
        relay.state = !relay.state;
        
        const payload = JSON.stringify({
            page: "device",
            value: {
                name: relay.name,
                status: relay.state ? "ON" : "OFF",
                gpio: parseInt(relay.gpio)
            }
        });
        
        Send_Data(payload);
        renderRelays(); // Re-render to update UI state
    }
}

function deleteRelay(id) {
    if(confirm("Delete this device?")) {
        relayList = relayList.filter(r => r.id !== id);
        renderRelays();
    }
}

// ==================== SETTINGS ====================
document.getElementById("settingsForm").addEventListener("submit", function (e) {
    e.preventDefault();
    const payload = JSON.stringify({
        page: "setting",
        value: {
            ssid: document.getElementById("ssid").value,
            password: document.getElementById("password").value,
            token: document.getElementById("token").value,
            server: document.getElementById("server").value,
            port: document.getElementById("port").value
        }
    });
    Send_Data(payload);
    alert("Configuration sent to device!");
});