<p align="center">
  <a href="https://youtu.be/9JrFDMzFvnI">
    <img src="https://github.com/gustavotorr/Engineering-Portfolio/blob/5ac5bbe5fecd9bf43e837780f4e253bbd4ab8b72/ball-and-beam-pid/Media/Photos/ball%20%26%20beam%20pid%20robot.png"
         width="85%"
         style="border-radius:14px;">
  </a>
</p>

<p align="center"><b>▶ Click image to watch demo video on YouTube</b></p>

# Ball-and-Beam PID Controller

A real-time PID control system that balances a ball on a beam using distance sensing and servo actuation. Features live tuning via joystick interface and OLED display feedback.

![System Status](https://img.shields.io/badge/status-stable-brightgreen) ![Arduino](https://img.shields.io/badge/platform-Arduino-00979D)

## 🎯 Project Overview

This project implements a classic control systems problem: maintaining a ball at a target position on a tilting beam. The system uses a VL53L0X Time-of-Flight sensor to measure ball position, processes the data through a PID controller, and adjusts beam angle via servo motor to maintain the desired setpoint.

### Key Features

- **Real-time PID Control** with 35ms sample time
- **Interactive Tuning** via analog joystick (no computer required)
- **Live Statistics** tracking error, failures, and performance
- **Smart Filtering** with moving average and jump detection
- **OLED Dashboard** showing system status and visual error bar
- **Robust Error Handling** for sensor timeouts and invalid readings

## 🛠️ Hardware Requirements

### Components

| Component | Model | Purpose |
|-----------|-------|---------|
| Microcontroller | Arduino Mega/compatible | Main controller |
| Distance Sensor | VL53L0X Time-of-Flight | Ball position measurement |
| Servo Motor | Standard hobby servo | Beam angle control |
| Display | SSD1306 OLED (128×64) | User interface |
| Input | 2-axis Analog Joystick | Parameter tuning |
| Button | Momentary push button | System enable/pause |

### Wiring

```
VL53L0X:
  - VCC → 5V
  - GND → GND
  - SDA → SDA (Pin 20)
  - SCL → SCL (Pin 21)

SSD1306 OLED:
  - VCC → 5V
  - GND → GND
  - SDA → SDA (Pin 20)
  - SCL → SCL (Pin 21)

Servo:
  - Signal → Pin 30
  - VCC → 5V (or external power for high-torque servos)
  - GND → GND

Joystick:
  - VRx → A0
  - VRy → A1
  - SW → Pin 5 (with internal pullup)
  - VCC → 5V
  - GND → GND

Enable Button:
  - One side → Pin 4 (with internal pullup)
  - Other side → GND
```

## 📐 Mechanical Setup

- **Beam Length:** 32.2 cm
- **Valid Range:** 1.5 - 33.7 cm from sensor
- **Servo Flat Position:** 112° (adjust based on your servo)
- **Servo Range:** 25° - 180°

Mount the VL53L0X sensor at one end of the beam, pointing along its length. The servo should be positioned to tilt the beam, with the ball able to roll freely along the surface.

## 💻 Software Setup

### Dependencies

Install these libraries via Arduino Library Manager:

```cpp
- Servo (built-in)
- Wire (built-in)
- Adafruit_GFX
- Adafruit_SSD1306
- VL53L0X (Pololu)
```

### Installation

1. Clone this repository:
```bash
git clone https://github.com/gustavotorr/ball-beam-pid.git
```

2. Open `Ball_On_Beam_PID.ino` in Arduino IDE

3. Install required libraries (see above)

4. Upload to your Arduino board

## 🎮 Usage

### Basic Operation

1. **Power On:** System initializes and displays "Booting System..."
2. **Press Enable Button:** Starts PID control (shows "RUNNING")
3. **Press Enable Again:** Pauses system (shows "PAUSED")

### Dashboard Display

```
STATUS: >> RUNNING
DIST: 18.3 cm
TARG: 18.0 cm
[========|--------] ← Visual error bar
```

### Menu System

**Access Menu:** Press joystick button from dashboard

**Navigate:** Move joystick up/down

**Edit Parameter:** Press button on Kp, Ki, Kd, or Target
- Move joystick left/right to adjust value
- Press button again to save

**Menu Options:**
- `Kp` - Proportional gain (adjust ±0.1)
- `Ki` - Integral gain (adjust ±0.01)
- `Kd` - Derivative gain (adjust ±0.1)
- `Target` - Target distance in cm (adjust ±0.5)
- `RESET` - Restore default PID values
- `EXIT` - Return to dashboard

## 🔧 Tuning Guide

### Default PID Values

```cpp
Kp = 6.2   // Proportional gain
Ki = 0.15  // Integral gain
Kd = 3.1   // Derivative gain
Target = 18.0 cm  // Setpoint
Deadband = 1.0 cm // No-control zone
```

### Tuning Process

1. **Start with Defaults:** Run system and observe behavior
2. **Adjust Kp:** 
   - Too low → slow response
   - Too high → oscillation
3. **Adjust Kd:** 
   - Dampens oscillations
   - Higher values reduce overshoot
4. **Adjust Ki:** 
   - Eliminates steady-state error
   - Too high causes instability

### Performance Monitoring

The system tracks and displays via Serial Monitor (9600 baud):

**Real-time Data (every 5 loops):**
```
18.3 | 18.0 | -0.3 | P:-1.9 I:0.2 D:0.5 | -1.2 | 125 | CTRL
```

**Statistics (every 500 loops):**
```
--- STATS ---
Loops: 500 | Sensor Fails: 0 | Jump Rejects: 2
Max Error: 3.2 cm | Avg Error: 0.45 cm
```

## 📊 System Architecture

### Control Loop (35ms cycle)

```
1. Read Distance Sensor
   ↓
2. Validate & Filter Reading
   ↓
3. Calculate PID Output
   ↓
4. Constrain & Apply to Servo
   ↓
5. Update Statistics
```

### Safety Features

- **Jump Rejection:** Ignores readings that change >6.0 cm between samples
- **Range Validation:** Discards readings outside 1.5-33.7 cm
- **Timeout Detection:** Handles sensor communication failures
- **Output Limiting:** Constrains servo angle to safe range
- **Integral Windup Protection:** Limits integral term accumulation

### Filtering

- **Moving Average:** 7-sample buffer smooths sensor noise
- **Derivative Filter:** Low-pass filter (α=0.6) on D-term reduces high-frequency noise

## 🐛 Troubleshooting

| Problem | Possible Cause | Solution |
|---------|---------------|----------|
| "SENSOR FAILED!" on startup | Wiring or I2C issue | Check VL53L0X connections, verify I2C address |
| Ball oscillates wildly | Kp too high | Reduce proportional gain |
| Slow response | Kp too low or Kd too high | Increase Kp, decrease Kd |
| Steady-state error | Ki too low | Increase integral gain |
| "! SAT" status | Output saturated | Check mechanical range, reduce gains |
| Intermittent sensor failures | Power supply issue | Use external power for servo |

## 📈 Performance Characteristics

- **Sample Rate:** ~28.6 Hz (35ms period)
- **Sensor Range:** 1.5 - 33.7 cm
- **Measurement Accuracy:** ±1-2mm (typical)
- **Settling Time:** ~2-4 seconds (with default tuning)
- **Steady-State Error:** <1.0 cm (within deadband)

## 📝 License

This project is open source and available under the [MIT License](LICENSE).

## 👤 Author

**Your Name**
- GitHub: [@gustavotorr](https://github.com/gustavotorr)
- LinkedIn: [Gustavo Torres](https://www.linkedin.com/in/gustavo-torres111/)

## 🙏 Acknowledgments

- VL53L0X library by Pololu
- Adafruit for display libraries
- Arduino community for inspiration

---

⭐ Star this repo if you find it helpful!
