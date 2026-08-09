**WiFi Weather Station**


ESP8266 NodeMCU · BME280 · INA219 · MQ-2 · IR Sensor · OLED · Solar Power

| Purpose Solar-powered outdoor weather station that reads temperature, humidity, pressure, air quality, and power metrics, displays them on an OLED, and pushes data to the cloud every 30 seconds. | Build Time \~4 hours totalHour 1: WiringHour 2: IDE \+ LibrariesHour 3: Flash \+ DebugHour 4: Board cleanup |
| :---- | :---- |

# **Section 1: Components Overview**

This project uses 10 components. Each is explained below with its role, operating voltage, and how it interfaces with the ESP8266 NodeMCU.

## **1.1  ESP8266 NodeMCU**

The ESP8266 NodeMCU is the brain of the entire system. It is a Wi-Fi enabled microcontroller development board based on the ESP-12E module. It runs at 3.3V logic and has built-in Wi-Fi (802.11 b/g/n), making it perfect for IoT cloud data pushing without any external Wi-Fi shield.

### **Key Specifications**

| Spec | Value | Notes |
| :---- | :---- | :---- |
| **CPU** | Tensilica Xtensa LX106 @ 80 MHz | Can be overclocked to 160 MHz |
| **Flash** | 4 MB | Stores firmware \+ file system |
| **RAM** | 80 KB (usable \~35 KB) | Shared with Wi-Fi stack |
| **GPIO** | 11 usable digital I/O pins | D0–D8, plus TX, RX |
| **ADC** | 1 × 10-bit (A0) | 0–1 V input range (NOT 3.3V\!) |
| **I2C** | Software I2C on any pins | Typically D1=SCL, D2=SDA |
| **Logic voltage** | 3.3 V | NOT 5V tolerant on GPIO |
| **Operating current** | \~80 mA active, \~20 µA deep sleep | Wi-Fi TX peaks at 300 mA |
| **USB chip** | CH340 or CP2102 | Drivers needed on some OS |

| ⚠ WARNING: The A0 (ADC) pin on ESP8266 NodeMCU accepts a maximum of 1.0 V, not 3.3V. Connecting a higher voltage will permanently damage the ADC. Always use a voltage divider on analog sensor outputs. |
| :---- |
| **⚠ WARNING:** GPIO pins are 3.3V logic only. Do NOT connect 5V signals directly. The MQ-2 sensor runs on 5V but its analog output must be divided down to ≤1V before reaching A0. |

## **1.2  BME280 – Temperature, Humidity & Pressure Sensor**

The BME280 is a precision environmental sensor from Bosch. It measures three parameters simultaneously: ambient temperature (°C), relative humidity (%RH), and barometric pressure (hPa). It communicates via I2C (or SPI) and operates on 3.3V, making it a perfect direct connection to the ESP8266.

### **Key Specifications**

| Spec | Value | Notes |
| :---- | :---- | :---- |
| **Supply voltage** | 1.7 V – 3.6 V | Use 3.3V from NodeMCU directly |
| **Interface** | I2C or SPI | We use I2C |
| **I2C address** | 0x76 (SDO → GND) or 0x77 (SDO → VCC) | Default modules are 0x76 |
| **Temp range** | \-40°C to \+85°C | ±1.0°C accuracy |
| **Humidity range** | 0–100% RH | ±3% RH accuracy |
| **Pressure range** | 300–1100 hPa | ±1 hPa accuracy |
| **Current (normal)** | \~2.8 µA (1 Hz sampling) | Very low power |

### **Pin Connections to NodeMCU**

| Pin / Label | Connects To | Notes |
| :---- | :---- | :---- |
| **VCC** | 3V3 (NodeMCU) | 3.3V supply – DO NOT use 5V |
| **GND** | GND (NodeMCU) | Common ground |
| **SDA** | D2 / GPIO4 (NodeMCU) | I2C data line (shared bus) |
| **SCL** | D1 / GPIO5 (NodeMCU) | I2C clock line (shared bus) |
| **SDO** | GND | Sets I2C address to 0x76 |
| **CSB** | VCC | Forces I2C mode (not SPI) |

| 📌 NOTE: The BME280 and BMP280 look identical on most modules. The BME280 has humidity; the BMP280 does not. Check the chip marking under magnification. The code will fail silently if you have a BMP280 but try to read humidity. |
| :---- |
| **ℹ INFO:** All I2C devices (BME280, INA219, OLED) share the same two wires: SDA on D2 and SCL on D1. Each device has a unique address so they do not interfere with each other. Only connect SDA, SCL, VCC, and GND – no chip select needed for I2C. |

## **1.3  1.3-inch OLED Display (SH1106 / SSD1306)**

The 1.3-inch OLED is a 128×64 pixel monochrome display. It uses the SH1106 driver chip (most 1.3" modules) or the SSD1306 (most 0.96" modules). It communicates over I2C and runs on 3.3V. The display is self-emissive – no backlight needed – which keeps power consumption very low (\~20 mA active). It wakes only when the IR sensor detects motion (wave-to-wake), saving significant battery power.

### **Key Specifications**

| Spec | Value | Notes |
| :---- | :---- | :---- |
| **Screen size** | 1.3 inch diagonal | 128 × 64 pixels |
| **Driver chip** | SH1106 (1.3") or SSD1306 (0.96") | Different library initialization |
| **Interface** | I2C | 4-wire: VCC GND SDA SCL |
| **I2C address** | 0x3C (most modules) | Some use 0x3D – check with scanner |
| **Supply voltage** | 3.3V – 5V (most modules) | 3.3V from NodeMCU is fine |
| **Current** | \~20 mA when active | \~0 mA when blanked in software |
| **Viewing angle** | 160° | Clear from most angles |

### **Pin Connections to NodeMCU**

| Pin / Label | Connects To | Notes |
| :---- | :---- | :---- |
| **VCC** | 3V3 (NodeMCU) | Some modules also accept 5V |
| **GND** | GND (NodeMCU) | Common ground |
| **SDA** | D2 / GPIO4 (NodeMCU) | Shared I2C bus with BME280 & INA219 |
| **SCL** | D1 / GPIO5 (NodeMCU) | Shared I2C clock |

| 📌 NOTE: If the display shows noise or nothing at all, run the I2C scanner sketch to find the actual address. If it returns 0x3D instead of 0x3C, change the address in the U8g2 library initialization call. |
| :---- |
| **✅ TIP:** Power tip: When the IR sensor is not triggered, call u8g2.clearBuffer() followed by u8g2.sendBuffer() to blank the display. This drops OLED current consumption to near zero, extending battery life significantly. |

## **1.4  INA219 – Voltage & Current Monitor**

The INA219 is a zero-drift, bidirectional current/power monitor from Texas Instruments. In this project it is placed in series with the battery output to measure exactly how much current the solar panel is delivering and how much the ESP8266 system is consuming. This gives you live solar charging data in the cloud.

### **Key Specifications**

| Spec | Value | Notes |
| :---- | :---- | :---- |
| **Supply voltage** | 3.0 V – 5.5 V | Use 3.3V from NodeMCU |
| **Interface** | I2C | 4-wire connection |
| **I2C address** | 0x40 (default) | A0, A1 both to GND |
| **Voltage range** | 0 – 26 V (bus) | Measures the load supply rail |
| **Current range** | ±3.2 A | Default calibration on breakout boards |
| **Shunt resistor** | 0.1 Ω (on most breakouts) | Gives \~32 mV full-scale shunt voltage |
| **Resolution** | 0.8 mA | Depends on calibration register |

### **Pin Connections to NodeMCU**

| Pin / Label | Connects To | Notes |
| :---- | :---- | :---- |
| **VCC** | 3V3 (NodeMCU) | Powers the INA219 chip itself |
| **GND** | GND (NodeMCU) | Common ground |
| **SDA** | D2 / GPIO4 (NodeMCU) | Shared I2C bus |
| **SCL** | D1 / GPIO5 (NodeMCU) | Shared I2C clock |
| **Vin+** | Battery OUT+ (positive terminal) | Inline with load – current flows through here |
| **Vin−** | Step-up module IN+ (after shunt) | Negative end of shunt resistor |

| ℹ INFO: The INA219 measures current by reading the tiny voltage drop across an internal 0.1Ω shunt resistor. Vin+ and Vin− must be placed IN SERIES with the power path you want to measure. Do not connect them directly across the battery terminals in parallel. |
| :---- |
| **⚠ WARNING:** The shunt (Vin+ to Vin−) must carry the full load current. Keep the wires between battery, INA219, and step-up module as short and thick as possible to reduce resistance errors. |

## **1.5  MQ-2 Gas Sensor**

The MQ-2 is a metal oxide semiconductor (MOS) gas sensor sensitive to LPG, butane, propane, methane, alcohol, hydrogen, and smoke. It has a built-in heating coil that must reach operating temperature before readings are stable. The sensor produces an analog voltage on its AOUT pin proportional to gas concentration. It requires 5V for the heater circuit.

### **Key Specifications**

| Spec | Value | Notes |
| :---- | :---- | :---- |
| **Heater voltage** | 5V DC | Must be 5V – 3.3V gives unreliable readings |
| **Heater current** | \~150 mA | This is the biggest current draw in the system |
| **Output type** | Analog (AOUT) \+ Digital (DOUT) | We use analog for variable readings |
| **AOUT voltage range** | 0 – 5V (proportional to concentration) | Must be divided to ≤1V for ESP8266 A0 |
| **Warm-up time** | \~60 seconds | Readings before 60s are unreliable |
| **Detection range** | 300 – 10000 ppm | Varies by gas type |

### **Pin Connections to NodeMCU**

| Pin / Label | Connects To | Notes |
| :---- | :---- | :---- |
| **VCC** | 5V (Step-up module output) | CRITICAL: must be 5V, not 3.3V |
| **GND** | GND (NodeMCU / common) | Common ground |
| **AOUT** | Voltage divider → A0 (NodeMCU) | DO NOT connect AOUT directly to A0 |
| **DOUT** | Not used (optional D5 for alarm) | Digital threshold output |

### **Voltage Divider Circuit (MANDATORY)**

The MQ-2 AOUT can swing from 0V to 5V. The ESP8266 A0 pin can only accept 0V to 1.0V. You MUST add a voltage divider between AOUT and A0. Use two resistors of the same value (10kΩ each):

**Wiring:** MQ-2 AOUT → R1 (10kΩ) → junction point → R2 (10kΩ) → GND

**Connect:** Junction point (midpoint between R1 and R2) → NodeMCU A0

**Result:** 5V signal is halved to 2.5V... then halved again? No – the NodeMCU A0 has an internal voltage divider already (11kΩ / 220kΩ) that further reduces the voltage. With 10kΩ \+ 10kΩ external divider, the A0 sees ≤1.0V. This is the correct and safe configuration.

| ⚠ WARNING: NEVER connect MQ-2 AOUT directly to A0. Even at low gas concentrations the output can be 1–2V which WILL permanently damage the ESP8266 ADC input. The voltage divider is mandatory, not optional. |
| :---- |
| **⚠ WARNING:** The MQ-2 heater draws \~150mA at 5V. This is a significant load. Ensure your step-up module and battery can supply at least 500mA total (ESP8266 \+ MQ-2 \+ OLED combined). |

## **1.6  IR Sensor (Wave-to-Wake)**

The IR sensor (infrared obstacle/proximity detector) is used as a wake-up trigger for the OLED display. When you wave your hand in front of it, the display turns on for 15 seconds, then automatically blanks to save power. The sensor emits infrared light and detects its reflection from nearby objects. Most breakout modules have a comparator that outputs a clean digital LOW when an object is detected.

### **Key Specifications**

| Spec | Value | Notes |
| :---- | :---- | :---- |
| **Supply voltage** | 3.3V – 5V | Use 3.3V from NodeMCU |
| **Output type** | Digital (active LOW) | Pulls output to GND when object detected |
| **Detection range** | 2 cm – 30 cm | Adjustable via onboard potentiometer |
| **Output pin** | OUT (also labeled DO or SIG) | Connect to D6 / GPIO12 |
| **Current** | \~20 mA | Standard module with two LEDs |
| **Response time** | \<2 ms | Instant hand-wave detection |

### **Pin Connections to NodeMCU**

| Pin / Label | Connects To | Notes |
| :---- | :---- | :---- |
| **VCC** | 3V3 (NodeMCU) | Powers the IR emitter and receiver |
| **GND** | GND (NodeMCU) | Common ground |
| **OUT / DO** | D6 / GPIO12 (NodeMCU) | Goes LOW when object (hand) detected |

| ℹ INFO: The sensor output is active-LOW: it reads HIGH (3.3V) normally, and drops to LOW (0V) when it detects a hand wave. The code reads digitalRead(D6) \== LOW to trigger the display. The NodeMCU pin D6 is configured with INPUT\_PULLUP so it stays HIGH by default even if the sensor is unplugged. |
| :---- |
| **📌 NOTE:** Adjust the small blue potentiometer on the IR sensor module to set detection distance. Turn clockwise to increase range, counter-clockwise to decrease. Set it to about 10–15cm so it triggers easily when you wave your hand but does not trigger from distant objects. |

## **1.7  Solar Panel (6V, 110×60 mm)**

The solar panel is the primary energy source for this weather station. It generates up to 6V open-circuit voltage in direct sunlight. The panel charges the 18650 lithium battery through the TP4056 charging module. The panel size (110×60 mm) is small but sufficient for slow, continuous trickle charging in outdoor daylight conditions.

### **Key Specifications**

| Spec | Value | Notes |
| :---- | :---- | :---- |
| **Open circuit voltage (Voc)** | \~6V in full sun | Drops to \~4-5V under load |
| **Short circuit current (Isc)** | \~100-150 mA | Depends on sunlight intensity |
| **Peak power** | \~0.6W | At maximum power point (Vmpp ≈ 5V) |
| **Physical size** | 110 × 60 mm | Compact, suitable for small enclosures |
| **Connection** | Red wire \= \+, Black wire \= − | Standard convention |

### **Wiring – Solar Panel to TP4056**

| Pin / Label | Connects To | Notes |
| :---- | :---- | :---- |
| **Solar \+ (red)** | TP4056 IN+ pin | Positive solar terminal to charger input |
| **Solar − (black)** | TP4056 IN− pin | Negative solar terminal to charger input |

| ℹ INFO: The solar panel is NOT connected to the ESP8266 directly. It feeds the TP4056 charger which manages safe lithium battery charging. The ESP8266 is powered by the battery through the step-up module. |
| :---- |
| **📌 NOTE:** Place the solar panel in a south-facing direction (in the Northern Hemisphere) at a 30-45° tilt for best year-round solar capture. Avoid shadows from nearby objects. |

## **1.8  TP4056 – Lithium Battery Charger Module**

The TP4056 is a complete constant-current/constant-voltage lithium-ion battery charger IC. The breakout module typically includes overvoltage, undervoltage, and overcurrent protection (the blue 4-pin modules lack protection; the blue 6-pin modules include it). It takes the solar panel's variable voltage and safely charges the 18650 cell to 4.2V maximum.

### **Key Specifications**

| Spec | Value | Notes |
| :---- | :---- | :---- |
| **Input voltage** | 4.5V – 5.5V (optimal 5V) | Solar panel at 6V is slightly above – most modules handle this |
| **Charge current** | 1A max (default) | Can be reduced via PROG resistor – 500mA recommended for solar |
| **Charge termination voltage** | 4.2V ± 1% | Standard Li-ion cutoff |
| **Battery protection** | Included on 6-pin modules | Prevents over-discharge (≤2.5V cutoff) |
| **LED indicators** | Red \= charging, Blue/Green \= charged | Useful for diagnostics |

### **Pin Connections**

| Pin / Label | Connects To | Notes |
| :---- | :---- | :---- |
| **IN+** | Solar Panel positive | Input from solar panel |
| **IN−** | Solar Panel negative / GND | Input ground |
| **B+** | 18650 Battery positive terminal | Battery connection positive |
| **B−** | 18650 Battery negative terminal | Battery connection negative |
| **OUT+** | Battery holder positive (also to Step-up IN+) | Load output |
| **OUT−** | Battery holder negative / GND | Load output ground |

| ⚠ WARNING: Use the 6-pin TP4056 module (has OUT+ and OUT− pads separate from B+ and B−) which includes battery protection. The 4-pin module lacks undervoltage protection which can permanently damage the lithium cell if drained completely. |
| :---- |
| **⚠ WARNING:** Do not exceed 6V on the IN+ pin. Some TP4056 modules tolerate up to 6.5V but this is outside the rated specification. If your solar panel voltage in full sun regularly exceeds 6V, add a diode (1N4007) in series with IN+ to drop the voltage slightly. |

## **1.9  Step-Up Voltage Module (3.7V → 5V, MT3608)**

The step-up (boost) converter takes the variable 3.2V–4.2V from the 18650 lithium battery and boosts it to a stable 5V DC output. The NodeMCU's VIN pin accepts 4.5V–10V and has an onboard AMS1117 LDO regulator that converts it down to 3.3V for the ESP8266 chip. The step-up module ensures the system runs reliably even when the battery is nearly depleted.

### **Key Specifications**

| Spec | Value | Notes |
| :---- | :---- | :---- |
| **IC** | MT3608 or similar | Common on cheap boost modules |
| **Input voltage** | 2V – 24V | Our battery range: 3.2V–4.2V |
| **Output voltage** | Adjustable via potentiometer | Set to exactly 5.0V before connecting |
| **Output current** | Up to 2A (MT3608) | More than enough for ESP8266 \+ MQ-2 |
| **Efficiency** | \~93% | Good efficiency at low currents |
| **Adjustment** | Small blue potentiometer on module | Turn clockwise to increase voltage |

### **Pin Connections**

| Pin / Label | Connects To | Notes |
| :---- | :---- | :---- |
| **IN+** | Battery OUT+ (from TP4056) | Battery positive |
| **IN−** | Battery OUT− / GND | Battery negative / common ground |
| **OUT+** | NodeMCU VIN AND MQ-2 VCC | 5V regulated output feeds both |
| **OUT−** | GND (common) | Output ground, connects to NodeMCU GND |

| ✅ TIP: Before connecting the step-up module to the NodeMCU, measure its output with a multimeter and adjust the potentiometer to exactly 5.0V. Too high damages the NodeMCU. Too low causes brownouts and Wi-Fi disconnections. |
| :---- |

## **1.10  18650 Li-ion Battery**

The 18650 is a cylindrical lithium-ion cell, the same chemistry used in laptop batteries and electric vehicles. It provides 3.7V nominal (3.2V depleted, 4.2V fully charged). In this project it acts as the energy buffer: it stores energy from the solar panel during the day and powers the weather station at night or on cloudy days.

### **Key Specifications**

| Spec | Value | Notes |
| :---- | :---- | :---- |
| **Nominal voltage** | 3.7V | Average discharge voltage |
| **Full charge voltage** | 4.2V | TP4056 charges to this exactly |
| **Minimum voltage** | 3.0V (protected cells: 2.5V cutoff) | Do not discharge below 3.0V |
| **Typical capacity** | 2000–3500 mAh | Depends on cell brand/quality |
| **Size** | 18mm diameter × 65mm length | Standard form factor |
| **Chemistry** | LiCoO2 or NMC | Standard li-ion, NOT LiFePO4 |

| ⚠ WARNING: NEVER short-circuit an 18650 cell. It can deliver hundreds of amperes and cause fire or explosion. Always use a protected cell or the 6-pin TP4056 module with protection circuit. |
| :---- |
| **⚠ WARNING:** Do not leave the battery charging unattended for extended periods without a proper protection circuit. Use only genuine cells from reputable brands (Panasonic, Samsung, LG, Sony). |

# **Section 2: Complete Pin Configuration**

The following table is the definitive wiring reference. Every wire in your build corresponds to exactly one row in this table. Print this page and keep it next to your varo board while wiring.

## **2.1  NodeMCU Pin Assignment Summary**

| Pin / Label | Connects To | Notes |
| :---- | :---- | :---- |
| **3V3** | BME280 VCC | Sensor power supply |
| **3V3** | OLED VCC | Display power supply |
| **3V3** | INA219 VCC | Monitor power supply |
| **3V3** | IR Sensor VCC | IR module power supply |
| **GND** | ALL component GND | Single common ground rail |
| **D1 / GPIO5** | BME280 SCL \+ INA219 SCL \+ OLED SCL | I2C clock – all 3 devices share this wire |
| **D2 / GPIO4** | BME280 SDA \+ INA219 SDA \+ OLED SDA | I2C data – all 3 devices share this wire |
| **A0** | MQ-2 voltage divider midpoint | Analog gas reading (max 1.0V input) |
| **D6 / GPIO12** | IR Sensor OUT (DO) | Digital motion detection |
| **D7 / GPIO13** | LED anode (optional) | Wake indicator LED via 220Ω resistor |
| **VIN** | Step-up OUT+ | 5V input to NodeMCU (do not use 3V3 pin for this) |

## **2.2  Power Chain Wiring**

| Pin / Label | Connects To | Notes |
| :---- | :---- | :---- |
| **Solar \+ (red)** | TP4056 IN+ | Solar energy in |
| **Solar − (black)** | TP4056 IN− | Solar ground |
| **TP4056 B+** | 18650 battery \+ | Charge the cell |
| **TP4056 B−** | 18650 battery − | Battery negative |
| **TP4056 OUT+** | Step-up module IN+ | Discharged battery power out |
| **TP4056 OUT−** | Step-up module IN− / GND | Common ground |
| **Step-up OUT+** | NodeMCU VIN AND MQ-2 VCC | 5V regulated power |
| **Step-up OUT−** | GND rail | Common ground |

## **2.3  INA219 Series Wiring Detail**

The INA219 must be placed IN SERIES with the power path to measure current. Place it between the battery output (TP4056 OUT+) and the step-up module input (Step-up IN+). Current flows: Battery → TP4056 OUT+ → INA219 Vin+ → INA219 shunt → INA219 Vin− → Step-up IN+.

| Pin / Label | Connects To | Notes |
| :---- | :---- | :---- |
| **TP4056 OUT+** | INA219 Vin+ | Current enters the shunt here |
| **INA219 Vin−** | Step-up IN+ | Current exits the shunt here, goes to load |
| **INA219 VCC** | NodeMCU 3V3 | Powers the INA219 chip |
| **INA219 GND** | GND rail | Common ground |
| **INA219 SDA** | D2 / GPIO4 | Shared I2C bus |
| **INA219 SCL** | D1 / GPIO5 | Shared I2C clock |

## **2.4  MQ-2 Voltage Divider Circuit**

Build this on your varo board near the A0 pin. Use two 10kΩ resistors:

1. Solder R1 (10kΩ) between MQ-2 AOUT and the junction point.

2. Solder R2 (10kΩ) between the junction point and GND.

3. Connect the junction point (between R1 and R2) to NodeMCU A0.

4. Connect MQ-2 GND to the common GND rail.

5. Connect MQ-2 VCC to the Step-up OUT+ (5V rail).

| ℹ INFO: With this divider: at MQ-2 AOUT \= 5V (maximum output), A0 sees approximately 0.83V which is safely below the 1.0V limit. The formula is: V\_A0 \= V\_AOUT × R2/(R1+R2) × \[correction for ESP8266 internal divider\] ≈ V\_AOUT × 0.167. |
| :---- |

# **Section 3: Arduino IDE Setup**

## **3.1  Install Arduino IDE**

6. Download Arduino IDE 2.x from https://arduino.cc/en/software

7. Install for your operating system (Windows/macOS/Linux).

8. Launch Arduino IDE.

## **3.2  Add ESP8266 Board Support**

9. Open Arduino IDE → File → Preferences (or Arduino → Settings on macOS).

10. In the 'Additional boards manager URLs' field, paste this URL exactly:

    http://arduino.esp8266.com/stable/package\_esp8266com\_index.json

11. Click OK to close Preferences.

12. Go to Tools → Board → Boards Manager.

13. Search for 'esp8266' and install the 'esp8266 by ESP8266 Community' package (version 3.1.x or latest).

14. After installation, go to Tools → Board → ESP8266 Boards → NodeMCU 1.0 (ESP-12E Module).

15. Set Tools → CPU Frequency → 80 MHz.

16. Set Tools → Flash Size → 4MB (FS:2MB OTA:\~1019KB).

17. Set Tools → Upload Speed → 115200\.

## **3.3  Install Required Libraries**

Install all these libraries via Sketch → Include Library → Manage Libraries:

| Library Name | Author | Purpose |
| :---- | :---- | :---- |
| **Adafruit BME280 Library** | By Adafruit | For temperature, humidity, pressure |
| **Adafruit Unified Sensor** | By Adafruit | Required dependency for BME280 |
| **Adafruit INA219** | By Adafruit | For voltage and current monitoring |
| **U8g2** | By oliver | OLED display driver (SH1106 \+ SSD1306 both) |
| **ThingSpeak** | By MathWorks | Cloud data upload library |

| 📌 NOTE: The U8g2 library covers both the SH1106 (1.3 inch) and SSD1306 (0.96 inch) OLED drivers. You select the correct one in the code by using the appropriate constructor. See Section 4 for details. |
| :---- |

## **3.4  ThingSpeak Account Setup**

18. Go to https://thingspeak.com and create a free account.

19. Click 'New Channel' and name it 'Weather Station'.

20. Enable and name these 8 fields: Temperature, Humidity, Pressure, Gas ADC, Bus Voltage, Current mA, Power mW, WiFi RSSI.

21. Click 'Save Channel'.

22. Go to the API Keys tab and copy the Write API Key.

23. Note your Channel ID (shown in the channel URL and header).

24. Paste both into the code at the top of the .ino file.

# **Section 4: Firmware – Code Walkthrough**

## **4.1  Configuration Block (Edit Before Uploading)**

At the very top of weather\_station.ino, edit these four values before uploading:

| Variable | What to enter | Notes |
| :---- | :---- | :---- |
| **WIFI\_SSID** | Your Wi-Fi network name | Case-sensitive |
| **WIFI\_PASSWORD** | Your Wi-Fi password | Case-sensitive |
| **CHANNEL\_ID** | Your ThingSpeak channel ID | Numeric, e.g. 2345678 |
| **WRITE\_KEY** | Your ThingSpeak Write API Key | 16-character alphanumeric string |

## **4.2  Pin Definitions**

These match the wiring in Section 2 exactly. Do not change unless you re-wire:

| Define | Physical Pin | Function |
| :---- | :---- | :---- |
| **PIN\_SCL \= D1** | GPIO5 | I2C clock |
| **PIN\_SDA \= D2** | GPIO4 | I2C data |
| **PIN\_MQ2 \= A0** | ADC input | Gas sensor analog |
| **PIN\_IR \= D6** | GPIO12 | IR sensor output |
| **PIN\_LED \= D7** | GPIO13 | Optional indicator LED |

## **4.3  OLED Driver Selection**

In the code, the OLED is initialized with this line:

U8G2\_SH1106\_128X64\_NONAME\_F\_HW\_I2C u8g2(U8G2\_R0, U8X8\_PIN\_NONE);

If your display shows nothing or random noise, your OLED may use SSD1306 instead of SH1106. Change the constructor to:

U8G2\_SSD1306\_128X64\_NONAME\_F\_HW\_I2C u8g2(U8G2\_R0, U8X8\_PIN\_NONE);

## **4.4  BME280 Forced Mode**

The BME280 is configured in FORCED mode, which means it takes a single measurement when requested, then returns to sleep. This is the lowest-power mode for weather monitoring. The code calls bme.takeForcedMeasurement() before reading data. In FORCED mode, the sensor is only awake for a few milliseconds per reading cycle, compared to NORMAL mode where it samples continuously.

## **4.5  Wave-to-Wake Logic**

The IR sensor logic works like this:

25. Every 500ms, digitalRead(PIN\_IR) is checked.

26. When a hand wave is detected (LOW signal), displayOn \= true and displayTimeout is set to now \+ 15,000 ms.

27. While displayOn is true, updateDisplay() is called every loop to refresh the OLED.

28. When millis() exceeds displayTimeout, displayOn is set to false, and the OLED is blanked.

This saves significant battery: the OLED draws \~20mA when active. With 15-second timeouts, display power is minimal over a typical day.

## **4.6  ThingSpeak Field Mapping**

| ThingSpeak Field | Sensor Data | Units/Source |
| :---- | :---- | :---- |
| **Field 1** | Temperature | °C from BME280 |
| **Field 2** | Humidity | %RH from BME280 |
| **Field 3** | Pressure | hPa from BME280 |
| **Field 4** | Gas ADC raw | 0–1023 from MQ-2 via A0 |
| **Field 5** | Bus Voltage | Volts from INA219 |
| **Field 6** | Current | mA from INA219 |
| **Field 7** | Power | mW from INA219 |
| **Field 8** | WiFi RSSI | dBm signal strength |

| 📌 NOTE: ThingSpeak free tier allows updates every 15 seconds minimum. Our 30-second interval is well within the limit. Do not set the interval below 15 seconds or ThingSpeak will start rejecting pushes with HTTP 401 errors. |
| :---- |

# **Section 5: Step-by-Step Build Process**

## **Hour 1 – Wiring the Varo Board**

29. Start with the power chain. Solder the solar panel wires to TP4056 IN+ and IN−.

30. Solder the 18650 battery holder to TP4056 B+ and B−.

31. Connect TP4056 OUT+ and OUT− to the step-up module IN+ and IN−.

32. Insert the INA219 in series: TP4056 OUT+ → INA219 Vin+ → INA219 Vin− → Step-up IN+.

33. Measure step-up output with multimeter. Adjust potentiometer to exactly 5.0V.

34. Connect Step-up OUT+ to NodeMCU VIN and to MQ-2 VCC.

35. Connect Step-up OUT− to common GND rail.

36. Run a common GND wire from NodeMCU GND to the GND rail.

37. Connect all 3V3 devices: BME280, OLED, INA219, IR sensor VCC to NodeMCU 3V3.

38. Connect all their GND pins to common GND rail.

39. Run the shared I2C bus: D1 (SCL) to BME280 SCL \+ INA219 SCL \+ OLED SCL.

40. Run the shared I2C bus: D2 (SDA) to BME280 SDA \+ INA219 SDA \+ OLED SDA.

41. Build voltage divider for MQ-2: two 10kΩ resistors, midpoint to A0.

42. Connect IR sensor OUT to D6.

43. Connect optional LED with 220Ω resistor from D7 to GND.

44. Double-check all connections against Section 2 table before powering on.

## **Hour 2 – Arduino IDE and Library Setup**

45. Install Arduino IDE 2.x (see Section 3.1).

46. Add ESP8266 board package URL and install (Section 3.2).

47. Install all 5 required libraries (Section 3.3).

48. Create ThingSpeak account and channel (Section 3.4).

49. Open weather\_station.ino, fill in WiFi credentials and ThingSpeak keys.

50. Select board: Tools → NodeMCU 1.0 (ESP-12E). Select correct COM/USB port.

## **Hour 3 – Upload, Debug, and Verify**

51. Connect NodeMCU to PC via USB cable.

52. Click Upload (→) in Arduino IDE. Wait for 'Done uploading'.

53. Open Serial Monitor (Tools → Serial Monitor). Set baud rate to 115200\.

54. Verify these messages appear: \[OK\] BME280, \[OK\] INA219, \[WiFi\] Connected.

55. Wave your hand in front of the IR sensor. OLED should wake up showing sensor data.

56. Wait 30 seconds. Check Serial Monitor for '\[Cloud\] Push OK'.

57. Log in to ThingSpeak and open your channel. You should see incoming data on all 8 fields.

58. Let the MQ-2 warm up for 60 seconds. Gas readings will stabilize.

| 🔍 I2C SCANNER: If BME280 or INA219 is not found, upload the I2C scanner sketch (available in Arduino IDE examples under Wire → i2c\_scanner). It will print all detected I2C addresses. Compare against expected: BME280=0x76, INA219=0x40, OLED=0x3C. |
| :---- |

## **Hour 4 – Final Assembly and Weatherproofing**

59. Once all components are verified working, tidy up wiring with zip ties or heat-shrink.

60. Add 10kΩ pull-up resistors on SDA (D2) and SCL (D1) lines to 3V3 if I2C is unstable.

61. Secure all components to the varo board with hot glue or standoffs.

62. Place the board in a weatherproof enclosure (IP65 rated).

63. Drill a small hole or window for the IR sensor to see through.

64. Mount the solar panel on top of or near the enclosure, south-facing.

65. Route a small cable gland for the solar panel wires.

66. Power on the final assembly with the battery installed.

67. Verify the system is reporting to ThingSpeak from its final location.

# **Section 6: Troubleshooting Guide**

| Problem | Solution | Root Cause |
| :---- | :---- | :---- |
| **BME280 not found** | Check I2C address: run I2C scanner. Check SDA/SCL not swapped. Verify VCC is 3.3V not 5V. | Most common cause: SDA and SCL wires swapped |
| **OLED shows noise** | Change U8G2 constructor from SH1106 to SSD1306 or vice versa. | Different 1.3" modules use different drivers |
| **INA219 reads 0 current** | Check Vin+ and Vin− are IN SERIES with the load, not across the battery. | Most common wiring mistake with INA219 |
| **MQ-2 reads maximum always** | Check voltage divider. Measure A0 pin voltage with multimeter – must be ≤1.0V. | ADC may be damaged if A0 exceeded 1V |
| **WiFi never connects** | Verify SSID/password. Check if 2.4GHz band (ESP8266 does not support 5GHz). Move closer to router. | ESP8266 supports 2.4GHz only |
| **ThingSpeak HTTP 401** | Check Write API Key. Channel must be public or key must match. Rate limit: min 15s between pushes. | Wrong key or too-frequent updates |
| **IR sensor always triggered** | Adjust potentiometer to reduce sensitivity. Check for reflective surfaces in detection path. | Background objects causing constant LOW |
| **Battery drains fast** | MQ-2 heater draws 150mA. Disable OLED when not in use. Use deep sleep between measurements. | MQ-2 is the biggest consumer |
| **Step-up output unstable** | Check input voltage from battery (must be ≥3.2V). Measure output with multimeter and readjust pot. | Battery too depleted or bad connection |
| **Upload fails** | Install CH340 or CP2102 driver for your OS. Check USB cable (some are charge-only, no data). | Driver issue is the most common upload problem |

# **Section 7: Power Budget Analysis**

Understanding how much power each component consumes helps you size the battery and solar panel correctly.

| Component | Current Draw | Notes |
| :---- | :---- | :---- |
| **ESP8266 (active \+ WiFi TX)** | \~80–300 mA @ 3.3V | Peaks at 300mA during WiFi transmission |
| **ESP8266 (idle, WiFi connected)** | \~70 mA @ 3.3V | Between transmissions |
| **BME280** | \~2.8 µA @ 3.3V | Negligible |
| **INA219** | \~1 mA @ 3.3V | Negligible |
| **OLED (active)** | \~20 mA @ 3.3V | Blanked when IR not triggered |
| **OLED (blanked)** | \~0 mA | Display off in software |
| **IR Sensor** | \~20 mA @ 3.3V | Always on |
| **MQ-2 (heater on)** | \~150 mA @ 5V \= 750 mW | Biggest consumer |
| **Step-up converter loss** | \~7% efficiency loss | Approximately 30–50 mW overhead |
| **TOTAL (worst case)** | \~300 mA peak, \~250 mA average | At 5V equivalent |

**Daily energy budget:** At 250mA average from a 5V equivalent system: 250mA × 24h \= 6,000 mAh at 5V \= \~16,200 mAh equivalent at 3.7V battery. A 2,500 mAh 18650 lasts approximately 10 hours without solar input. The 6V solar panel can deliver up to 100mA (about 500mW), which at moderate sun for 6h/day \= 3,000 mAh of charge at 3.7V – sufficient to maintain charge balance in good weather conditions.

| ✅ TIP: To dramatically extend battery life: add ESP8266 deep sleep between 30-second cloud pushes (draws only \~20µA during sleep), and turn off the MQ-2 heater using a MOSFET when measurements are not needed. |
| :---- |

# **Quick Reference – At a Glance**

| Item | Setting | Notes |
| :---- | :---- | :---- |
| **I2C Bus** | SDA=D2, SCL=D1 | Shared by BME280, INA219, OLED |
| **Gas Sensor** | A0 (with 10kΩ÷10kΩ divider) | MQ-2 AOUT must NOT exceed 1V on A0 |
| **IR Sensor** | D6 (GPIO12), INPUT\_PULLUP | Active LOW when hand detected |
| **MQ-2 Power** | 5V from Step-up module | NOT 3.3V |
| **NodeMCU Power** | 5V into VIN from Step-up | On-board AMS1117 makes 3.3V |
| **Cloud push interval** | 30 seconds | ThingSpeak minimum is 15 seconds |
| **Display wake time** | 15 seconds after IR trigger | Adjustable in code: DISPLAY\_ON\_MS |
| **Serial baud rate** | 115200 | For debugging in Serial Monitor |

