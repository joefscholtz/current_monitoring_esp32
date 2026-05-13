## Design Documentation

## 1. System Architecture

```mermaid
C4Container
    title Container & Hardware Interaction Diagram

    %% Physical Hardware Layer
    System_Boundary(hw, "Physical Hardware") {
        Container(leakage_ct, "CT 2000:1 (Leakage)", "Hardware", "GPIO 34 (ADC1_CH6) + 22k Burden")
        Container(load_ct, "CT 1000:1 (Load)", "Hardware", "GPIO 35 (ADC1_CH7) + 100R Burden")
        Container(eth_chip, "ENC28J60 (Ethernet)", "SPI Peripheral", "GPIO 18, 19, 23 (VSPI)")
    }

    %% Firmware Layer
    Container_Boundary(esp32, "ESP32 SoC") {

        Container_Boundary(core1, "Core 1 (Sensing)") {
            Container(sensor_task, "Sensor Task", "FreeRTOS", "vTaskDelayUntil (300ms frequency)")
            Container(cm_comp, "Current Monitor", "IDF Component", "ADC1 Drivers + RMS Engine")
        }

        Container_Boundary(core0, "Core 0 (Networking)") {
            Container(network_task, "Network Task", "FreeRTOS", "Asynchronous MQTT Client")
            Container(nm_comp, "Network Manager", "IDF Component", "WiFi/ETH/MQTT Handlers")
        }

        ContainerDb(queue, "Data Queue", "FreeRTOS Queue", "Current_readings_t (inc. Timestamp)")
    }

    %% Relationships
    Rel(leakage_ct, cm_comp, "Analog Signal", "Voltage (0-3.3V)")
    Rel(load_ct, cm_comp, "Analog Signal", "Voltage (0-3.3V)")
    Rel(sensor_task, cm_comp, "Reads Raw/RMS", "API Call")
    Rel(sensor_task, queue, "Pushes Data", "Queue Send (Non-blocking)")

    Rel(queue, network_task, "Pops Data", "Queue Receive (Blocking)")
    Rel(network_task, nm_comp, "Triggers Pub", "API Call")
    Rel(nm_comp, eth_chip, "SPI Commands", "VSPI Bus")
```

## 3. Design Decisions

### 3.1 RMS Sampling Strategy

To meet the 300ms maximum interval requirement, the firmware implements a high-frequency sampling window:

- Sampling Frequency: ~5-10kHz per channel to accurately capture 60Hz waveforms.
- RMS Calculation: Discrete-time True RMS calculation is performed: $V_{rms} = \sqrt{\frac{1}{N} \sum_{i=1}^{N} (v_i - V_{offset})^2}$
- Fixed Frequency: Using `vTaskDelayUntil` ensures the 300ms deadline is strictly met regardless of calculation overhead, avoiding drift.

### 3.2 Noise Handling

- Hardware Offset: A 1.65V DC bias is applied to the CT signal to allow the ESP32 (0-3.3V range) to sample the AC negative half-cycle.
- Software Filtering:
  - Deadzone: A small software threshold is applied to eliminate "ghost" readings caused by ADC noise floor.
  - Averaging: Multiple samples are accumulated per window to smooth out transient spikes.
  - Hardware Calibration: ADC calibration to mitigate chip-to-chip gain errors.

### 3.3 Software Architecture

- Modular Component Architecture: Logic is segregated into reusable, standalone components (`current_monitor`, `network_manager`) to minimize coupling and facilitate parallel testing.
- Provider/Consumer Model: The `apps/` act as consumers of the `components/` provider library. This allows the same hardware-interfacing code to be reused across different execution models (Bare-Metal Super-Loop vs. RTOS Preemptive Tasks).
- Opaque Handle Pattern: By using pointer-based handles for component initialization and interaction, the internal structures remain protected from the application layer, reducing the risk of accidental memory corruption.
- Decoupling (Dual-Core): Sampling is pinned to Core 1 to ensure zero jitter.
- Networking (WiFi/MQTT) is pinned to Core 0 to handle the non-deterministic TCP/IP stack.
- Unified Identity: Both Wi-Fi and Ethernet use the internal Factory MAC as the MQTT Client ID, ensuring the backend treats the device as a single entity regardless of the physical connection.
