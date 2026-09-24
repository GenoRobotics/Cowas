# CoWaS pin map (Arduino Due)

Generated in FW0 from `include/Settings.h` and the code at commit `d508f41` + FW0. HW1 fills in the three empty columns on the real machine.

**The Due is 3.3 V only: no pin is 5 V tolerant.** "3V3 out" means the Due drives 0 / 3.3 V. "≤ 3.3 V in" means whatever is wired there must never exceed 3.3 V.

- **Reachable?**: does the PCB shield bring this pin out to a connector or pad (y/n, which connector)?
- **Measured**: the level measured in HW1 (idle / active), or "n/c".
- **Flags** point to the lists at the bottom: **U** = declared but unused, **X** = used without a Settings.h constant, **!** = hazard to check in HW1.

Line numbers are for the FW0 tree. "boot" means the pin is configured in `setup()` on every boot, whatever the mode.

## Digital pins D0–D53

| Pin | Due function | Constant (Settings.h) | Used by (file:line) | Dir | Expected level | Flags | Reachable on the PCB shield? | Measured level | Notes |
|---|---|---|---|---|---|---|---|---|---|
| D0 | RX0 (Programming port, via 16U2) | — | `Serial` console + legacy Pi link: `C_output.cpp:30`, `main.cpp:202`, reads `main.cpp:254`, `Tests.cpp:87` | in | ≤ 3.3 V in | X1 | | | |
| D1 | TX0 (Programming port, via 16U2) | — | same as D0 | out | 3V3 out | X1 | | | |
| D2 | PWM, TIOA0 | `MOTOR_PWM1_PIN` 176 | VNH5019 M1 PWM (spool): `Motor.cpp:14`, `md.init()` via `Motor.cpp:37` (boot, `main.cpp:141`) | out | 3V3 PWM | U3 | | | |
| D3 | PWM | `MOTOR_PWM2_PIN` 181 | VNH5019 M2 PWM (manifold motor): `Motor.cpp:19`, `md.setM2Speed` `Motor.cpp:80,85` | out | 3V3 PWM | | | | |
| D4 | PWM, SPI NPCS1 (alt) | — | unused | — | — | | | | Tied to two SAM3X pins (PA29 + PC26) on the Due board |
| D5 | PWM | `MOTOR_EN2DIAG2_PIN` 182 | VNH5019 M2 EN/DIAG: set INPUT by `md.init()` (boot); never read | in | ≤ 3.3 V in; pulled up on the shield | U4, !3 | | | |
| D6 | PWM | `MOTOR_INA2_PIN` 179 | VNH5019 M2 INA (manifold): `Motor.cpp:17` | out | 3V3 out | | | | |
| D7 | PWM | `MOTOR_INB2_PIN` 180 | VNH5019 M2 INB (manifold): `Motor.cpp:18` | out | 3V3 out | | | | |
| D8 | PWM | `PRESSURE1_PIN` 154 | ABP (Trustability) SPI chip select, `pressure1`: `main.cpp:134`, `Trustability_ABP_Gage.cpp:35,62,66` | out | 3V3 out, idle HIGH | | | | **No sensor answers on this CS** (FW0 bench: `invalid SPI response`, F50). Becomes `p_in` in FW3 |
| D9 | PWM | `LINEAR_ACT_STEP_PIN` 197 | A4988 STEP: `main.cpp:170`, `Linear_actuator.cpp:35,91,93` | out | 3V3 out | | | | |
| D10 | PWM, SPI NPCS0 (alt) | `LINEAR_ACT_DIR_PIN` 198 | A4988 DIR: `main.cpp:170`, `Linear_actuator.cpp:36,78` | out | 3V3 out | | | | Tied to two SAM3X pins (PA28 + PC29) on the Due board; `SPI.begin()` without a pin does not claim it |
| D11 | PWM | `LINEAR_ACT_ENABLE_PIN` 199 | A4988 ENABLE (active LOW): `main.cpp:170`, `Linear_actuator.cpp:37,49,55` | out | 3V3 out | !4 | | | No external pull-up yet (Settings.h TODO): floats until `begin()` |
| D12 | PWM | `FLOW_BIG_PIN` 188 | `flow_sensor_big` pulse input + interrupt: `main.cpp:161`, `Flow_sensor.cpp:42,58`; volume printed only in `Step_functions.cpp:212` | in | ≤ 3.3 V in | U5 | | | Is a second flowmeter wired here? ROADMAP §11.2 plans `Q_out` on D12 (P2) |
| D13 | PWM, TIOB0, on-board LED "L" | `FLOW_SMALL_PIN` 186 | `flow_sensor_small` pulse input + interrupt: `main.cpp:160`, `Flow_sensor.cpp:42,58`; volume stop of every sample (`Pump.cpp:337-338`) | in | ≤ 3.3 V in | !1 | | | F26: the comment at d508f41 said "probably fried"; the MW-FS-2.0 swings to its supply. The on-board LED buffer also sits on this pin |
| D14 | TX3 | — | unused | — | — | | | | |
| D15 | RX3 | — | unused | — | — | | | | |
| D16 | TX2 | — | unused | — | — | | | | |
| D17 | RX2 | — | unused | — | — | | | | |
| D18 | TX1 | — | `Serial1` in `Serial_device.cpp:21-81` (Wi-Fi card link), never instantiated | — | — | X2 | | | Dead code today |
| D19 | RX1 | — | same as D18 | — | — | X2 | | | Dead code today |
| D20 | SDA (Wire) | — | unused | — | 1.5 kΩ on-board pull-up to 3.3 V | | | | ROADMAP §11.2: INA226 (P1) |
| D21 | SCL (Wire) | — | unused | — | 1.5 kΩ on-board pull-up to 3.3 V | | | | ROADMAP §11.2: INA226 (P1) |
| D22 | | `GREEN_LED_PIN` 153 | `green_led`: `main.cpp:133`, `Led.cpp:10,23,29,36` | out | 3V3 out | | | | |
| D23 | | `STATUS_LED_PIN` 152 | `status_led`: `main.cpp:132`, `Led.cpp:10,23,29,36` | out | 3V3 out | | | | |
| D24 | | `BUTTON_START_PIN` 168 | `button_start` (normally open): `main.cpp:146`, `Button.cpp:14,33`; panel mode switch `main.cpp:520` | in | ≤ 3.3 V in; plain INPUT, needs an external pull resistor | !2 | | | |
| D25 | | `BUTTON_LEFT_PIN` 172 | `button_left` (normally open): `main.cpp:144`, `Button.cpp:14,33` | in | as D24 | !2 | | | |
| D26 | | `BUTTON_RIGHT_PIN` 173 | `button_right` (normally open): `main.cpp:145`, `Button.cpp:14,33` | in | as D24 | !2 | | | |
| D27 | | `BUTTON_CONTAINER_PIN` 169 | `button_container` (normally closed): `main.cpp:147`; read in `step_fill_container()` `Step_functions.cpp:157`, `maintenance.cpp:275` | in | as D24 | U6, !2 | | | The container was replaced by the flowmeter |
| D28 | | `BUTTON_SPOOL_UP` 170 | spool up end-stop (normally closed): `main.cpp:148`, interrupt `main.cpp:150` → `ISR_emergency_stop_up` `Motor.cpp:230` | in | as D24 | U3, !2 | | | The ISR stays attached although the spool is disabled |
| D29 | | `BUTTON_SPOOL_DOWN` 171 | spool down end-stop: `main.cpp:151`; its interrupt is commented out (`main.cpp:153`) | in | as D24 | U3, !2 | | | |
| D30 | | `MOTOR_EN1DIAG1_PIN` 177 | VNH5019 M1 EN/DIAG (spool): INPUT via `md.init()`; read only in `Motor::stopIfFault()` `Motor.cpp:218`, which nothing calls | in | ≤ 3.3 V in; pulled up on the shield | U3, U4, !3 | | | |
| D31 | | `ENCODER_A_PIN` 165 | spool quadrature encoder A: `main.cpp:143`, `Encoder.cpp:16,75,77` | in | ≤ 3.3 V in | U3 | | | |
| D32 | | `MOTOR_INA1_PIN` 174 | VNH5019 M1 INA (spool): `Motor.cpp:12` | out | 3V3 out | U3 | | | |
| D33 | | `ENCODER_B_PIN` 166 | spool encoder B: `main.cpp:143`, `Encoder.cpp:17,80` | in | ≤ 3.3 V in | U3 | | | |
| D34 | | `MOTOR_INB1_PIN` 175 | VNH5019 M1 INB (spool): `Motor.cpp:13` | out | 3V3 out | U3 | | | Settings.h:164 has a commented `PUMP_VACUUM = 34`: same pin |
| D35 | | `ENCODER_Z_PIN` 167 | spool encoder Z: `main.cpp:143`, `Encoder.cpp:18` | in | ≤ 3.3 V in | U3 | | | |
| D36 | | — | unused | — | — | | | | ROADMAP §11.2: `Q_in` fallback if D13 fails |
| D37 | | — | unused | — | — | | | | ROADMAP §11.2: E-stop input (FW6) |
| D38 | | `ENCODER_MANIFOLD` 185 | AMT22 SPI chip select: `Manifold.cpp:75-76` (boot), `Manifold.cpp:350` via `spiWriteRead` | out | 3V3 out, idle HIGH | | | | |
| D39 | | — | unused | — | — | | | | ROADMAP §11.2: O-ring "unlocked" home switch (P1) |
| D40 | | `VALVE_23_PIN` 158 | `valve_23` (3/2 valve): `main.cpp:136`, `Valve_3_2.cpp:19,40,48,62` | out | 3V3 out (to a driver stage) | U6 | | | **The valve is physically removed** (F51); only its relay remains and still clicks. Code removal: FW4 |
| D41 | | — | unused | — | — | | | | ROADMAP §11.2: leak sensor (P1) |
| D42 | | `PUMP_ENABLE` 161 | 24 V pump relay: `main.cpp:138`, `Pump.cpp:29,104,156` | out | 3V3 out (to the relay driver) | | | | The only thing that really stops the pump (F25) |
| D43 | | — | unused | — | — | | | | ROADMAP §11.2: DS18B20 (P1) |
| D44 | | `VALVE_1_PIN` 157 | `valve_1` (2/2 valve, air inlet): `main.cpp:135`, `Valve_2_2.cpp:9,27,34` | out | 3V3 out (to a driver stage) | | | | |
| D45 | | — | unused | — | — | | | | ROADMAP §11.2: `P_out` CS (P2) |
| D46 | | `VALVE_MANIFOLD` 159 | `valve_manifold` (2/2 valve, deployment line): `main.cpp:137`, `Valve_2_2.cpp:9,27,34` | out | 3V3 out (to a driver stage) | | | | |
| D47 | | — | unused | — | — | | | | ROADMAP §11.2: `Q_out` fallback (P2) |
| D48 | | `ON_OFF_33V` 163 | micro pump (DNA/RNA Shield) through a transistor: `main.cpp:140`, `Micro_pump.cpp:17,40,72` | out | 3V3 out | | | | |
| D49 | | — | unused | — | — | | | | |
| D50 | | — | unused | — | — | | | | Plain GPIO on the Due (SPI is on the 6-pin header, not here) |
| D51 | | — | unused | — | — | | | | |
| D52 | SPI NPCS2 (alt) | — | unused | — | — | | | | |
| D53 | | — | unused | — | — | | | | |

## Analog inputs A0–A11 (D54–D65)

| Pin | Constant (Settings.h) | Used by (file:line) | Dir | Expected level | Flags | Reachable on the PCB shield? | Measured level | Notes |
|---|---|---|---|---|---|---|---|---|
| A0 | — | unused | — | — | | | | ROADMAP §11.2: battery voltage divider (P1) |
| A1 | `pressure_3_pin` 156 | nothing | — | — | U1 | | | "0–12 bar, cheaper one, only water" |
| A2 | `pressure_2_pin` 155 | `pressure2` (BigPressure, analog): `main.cpp:163`, `Pressure_sensor.cpp:34,39`; **backs the pump safety cap and every PID loop** via `read_pressure2()` `main.cpp:116-119`, `main.cpp:139`, `Step_functions.cpp:236,289,295,369,383,481` | in | 0–3.3 V (PCB divider 1 k / 680 Ω, ×1.68, `Pressure_sensor.cpp:59-62`) | !5 | | | **Sensor physically removed** (F50): the pin floats (read 0.11 bar at the FW0 bench check). Code deleted in FW3 |
| A3 | — | unused | — | — | | | | |
| A4 | — | unused | — | — | | | | |
| A5 | — | unused | — | — | | | | ROADMAP §11.2: turbidity (P2) |
| A6 | — | unused | — | — | | | | ROADMAP §11.2: leak probe option (P1) |
| A7 | — | unused | — | — | | | | |
| A8 | — | unused | — | — | | | | |
| A9 | — | unused | — | — | | | | |
| A10 | `MOTOR_CS1_PIN` 178 | VNH5019 M1 current sense (spool): INPUT via `md.init()`; never read | in | 0–3.3 V (0.14 V/A) | U3, U4 | | | |
| A11 | `MOTOR_CS2_PIN` 183 | VNH5019 M2 current sense (manifold): INPUT via `md.init()`; never read | in | 0–3.3 V (0.14 V/A) | U4 | | | |

## DAC, SPI, I²C, CAN

| Pin | Due function | Constant | Used by (file:line) | Dir | Expected level | Flags | Reachable on the PCB shield? | Measured level | Notes |
|---|---|---|---|---|---|---|---|---|---|
| DAC0 (D66) | DAC | — | unused | — | — | | | | |
| DAC1 (D67) | DAC | `PUMP_PIN` 160 | pump speed input: `main.cpp:138`, `analogWrite` `Pump.cpp:108,147` | out | **~0.55–2.75 V**, not 0–3.3 V | !6 | | | F25: "0 %" is still ~0.55 V; the relay on D42 stops the pump |
| MISO (SPI header) | SPI | — | ABP (`Trustability_ABP_Gage.cpp:63-65`) + AMT22 (`Manifold.cpp:376`); `SPI.begin()` `main.cpp:127` | in | ≤ 3.3 V in | X3, !7 | | | Two devices share the bus |
| MOSI (SPI header) | SPI | — | AMT22 commands (`Manifold.cpp:376`); ABP ignores it | out | 3V3 out | X3 | | | |
| SCK (SPI header) | SPI | — | ABP at 800 kHz (`Trustability_ABP_Gage.cpp:61`), AMT22 at the `SPI.begin()` default | out | 3V3 out | X3 | | | |
| SDA1 (D70) / SCL1 (D71) | I²C #2 | — | unused | — | no on-board pull-ups | | | | |
| CANRX (D68) / CANTX (D69) | CAN | — | unused | — | needs a transceiver | | | | |
| Native USB | SerialUSB | — | unused | — | — | | | | ROADMAP §3.4: the future host link (FW7) |

The code also sets up timers (not pins): `TC1` channels 0/1 with `TC3_Handler`/`TC4_Handler` (`Pump.cpp:362,383`, `main.cpp:129`), used only by the unreachable `test_2_*` functions in `Tests.cpp`.

## Summary

- **Used today:** D0/D1 (Serial), D2, D3, D5–D13, D22–D35, D38, D40, D42, D44, D46, D48, A2, A10, A11, DAC1, and the SPI header.
- **Free in the code:** D4, D14–D21, D36, D37, D39, D41, D43, D45, D47, D49–D53, A0, A1 (declared only), A3–A9, DAC0, SDA1/SCL1, CAN. The real limit is what the PCB shield brings out (column 7 above); the old "all pins are claimed" comment in Settings.h was wrong (F20).
- **Spool pins still configured at boot** although `SPOOL_USE = false`: D2, D28–D35, A10.

## Flags

**Declared but unused (U):**
- **U1** `pressure_3_pin = A1` (Settings.h:156): nothing reads it.
- **U3** Spool hardware (`SPOOL_USE = false`): D2, D28–D35 and A10 are still configured at boot, the D28 interrupt is attached (`main.cpp:150`), and `system_checkup()` would drive the spool motor, but it doesn't run at boot (F11).
- **U4** VNH5019 diagnostics: EN/DIAG (D5, D30) and current sense (A10, A11) are configured as inputs and never read (`Motor::stopIfFault()` has no caller). Motor faults and stalls go unseen.
- **U5** `FLOW_BIG_PIN = 12`: its interrupt counts pulses, but the value is only printed by `step_fill_container()`. Is anything wired?
- **U6** Container-era hardware: `BUTTON_CONTAINER_PIN` (D27) and `VALVE_23_PIN` (D40). The container is gone, but `step_fill_container()` still waits on D27 (up to `FILL_CONTAINER_TIME` = 11 min) and `valve_23` is still switched.
- `include/Flow_sensor.h:11-12` declares `flow_small = 12` and `flow_big = 13`, the **opposite** of Settings.h (`FLOW_SMALL_PIN = 13`, `FLOW_BIG_PIN = 12`). Nothing uses them, but they will mislead the next reader.
- `Encoder_atmel` (a TC0 quadrature decoder on the TIOA0/TIOB0 pins = D2/D13), `Potentiometer` (`extern` in `Step_functions.cpp:55`) and `GPIO wifi_message` (`extern` in `Serial_device.cpp:18`) are never instantiated. `Encoder_atmel` would clash with D2 (spool PWM) and D13 (flowmeter) if anyone enabled it.

**Used without a Settings.h constant (X):**
- **X1** D0/D1 (`Serial`): the human console and the legacy Pi link share this one port at 9600 baud (F10, F33).
- **X2** D18/D19 (`Serial1`) in `Serial_device.cpp`, dead code (no `Serial_device serial` object exists).
- **X3** The SPI header pins, implicit through `SPI.begin()` (`main.cpp:127`).

**Hazards to check in HW1 (!):**
- **!1** D13 flowmeter output level (F26). Powered at 5 V, the MW-FS-2.0 puts 5 V on a 3.3 V pin.
- **!2** Buttons and switches use `INPUT` with no internal pull-up: check that the external pull resistors go to **3.3 V, not 5 V**.
- **!3** VNH5019 EN/DIAG pull-ups: on the Pololu shield they go to the shield's logic supply. Check that D5/D30 never see more than 3.3 V.
- **!4** A4988 ENABLE (D11) has no pull-up, so the motor can twitch at power-up (Settings.h TODO: 10 kΩ to 3.3 V).
- **!5** A2 reads a sensor that was removed (F7/F22, F50). The pump cap and all PID loops act on a floating pin until FW3, and no other pressure sensor is installed.
- **!6** DAC1 range and fragility (F25): buffer it; never rely on "0 %".
- **!7** AMT22 MISO level and the shared SPI bus (F36).

## I²C addresses

None used today. List every I²C device here before ordering it (ROADMAP §11.3).

| Bus | Address | Device | Status |
|---|---|---|---|
| | | | |
