# Sleep Detection System Using EOG Signals with BioAmp EXG Pill and an Arduino UNO
**Authors:** V Sai Pallavi, Ahaan Shah, Riddhiman Ganguly, Varshini


## Overview

Sleep detection has emerged as an important area of research with significant applications in health monitoring, driver safety, and the development of wearable technology. Traditional methods, such as polysomnography, which integrates signals from electroencephalogram (EEG), electrooculography (EOG), electromyography (EMG), and other measures, are invasive and typically confined to clinical settings, posing challenges for widespread adoption due to their cost and complexity. With the vision of creating a wearable sleep detection solution, this project explores an novel approach using EOG to enhance accessibility and practicality.

## How it works

1. **Calibration** — On startup, the user blinks naturally until 15 blinks are detected. The system records the variance of each blink and the longest pause between blinks.
2. **Thresholding** — From calibration data, it derives:
   - a **variance threshold** (a fraction of the average blink variance) used to detect future blinks
   - a **sleep threshold** (2x the longest observed pause, clamped to 10–30s) — how long without a blink counts as "asleep"
3. **Signal processing** — Raw EOG samples (75 Hz) are smoothed with a 5-point moving average, then variance is computed over a rolling 1-second (75-sample) window to detect blink events.
4. **Detection loop** — If time since the last blink exceeds the sleep threshold, the system flags "Asleep" and triggers an alert; a new blink reverts it to "Awake."

## Hardware
![Block Diagram](images/block_diagram.png)
![Circuit Diagram](images/circuit_diagram.png)
- Arduino Uno
- BioAmp EXG Pill (EOG electrodes placed around the eye)
- Active buzzer
- Jumper wires

## Usage

1. Flash `sleep_detection.ino` to the Arduino Uno via the Arduino IDE.
2. Power on — the device begins calibration automatically.
3. Blink naturally 15 times when prompted (prompts appear over the Serial Monitor at 115200 baud).
4. After calibration completes, the system runs continuously, alerting when drowsiness is detected.

## Results

The sleep detection system was built and tested using the BioAmp EXG Pill connected to an Arduino Uno, with a speaker module providing audio feedback. We conducted experiments to evaluate its ability to detect drowsiness through calibration and continuous operation.
The model worked well for detecting the ‘Awake’ and ‘Asleep’ states of the user.
The alarm sound from the speaker module was loud and clear to alert the user when they fall asleep.


## Future work

- Miniaturization and power optimization for a true wearable form factor
- Additional sensor fusion (e.g., EEG/EMG) for broader sleep-stage analysis
- Adaptive alert tuning for different environments

## References

A. Sano, W. Chen, D. Lopez-Martinez, S. Taylor and R. W. Picard, "Multimodal Ambulatory Sleep Detection Using LSTM Recurrent Neural Networks," *IEEE Journal of Biomedical and Health Informatics*, vol. 23, no. 4, pp. 1607–1617, July 2019. doi: 10.1109/JBHI.2018.2867619
