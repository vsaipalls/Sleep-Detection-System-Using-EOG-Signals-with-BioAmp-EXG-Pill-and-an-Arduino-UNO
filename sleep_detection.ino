// Continuous Awake vs. Asleep Detection with BioAmp EXG Pill and Active Buzzer
// Custom calibration for 15 blinks, buzzer sounds until subject wakes up

// Pin definitions
const int eogPin = A0;    // BioAmp EXG Pill output to A0
const int ledPin = 13;    // Built-in LED: ON = Awake, OFF = Asleep
const int speakerPin = 9; // Active buzzer on pin 9

// Sampling parameters
const int SAMPLE_RATE = 75;          // 75 Hz sampling
const int SAMPLE_DELAY = 1000000 / SAMPLE_RATE; // 13.33 ms in microseconds
const int WINDOW_SIZE = 75;          // 1-second window (75 samples at 75 Hz)

// Smoothing parameters
const int SMOOTH_SIZE = 5;          // 5-point moving average
float smoothBuffer[SMOOTH_SIZE];
int smoothIndex = 0;

// Variance calculation buffer
float window[WINDOW_SIZE];
int windowIndex = 0;

// Calibration and runtime variables
float varianceThreshold = 0;    // Minimum variance for a blink
unsigned long SLEEP_THRESHOLD = 0; // Time without blinks = asleep
unsigned long lastBlinkTime = 0;  // Time of last detected blink
bool isAwake = true;              // Current state
bool calibrated = false;

// Smoothing function
float smoothSignal(float newValue) {
  smoothBuffer[smoothIndex] = newValue;
  smoothIndex = (smoothIndex + 1) % SMOOTH_SIZE;
  float sum = 0;
  for (int i = 0; i < SMOOTH_SIZE; i++) {
    sum += smoothBuffer[i];
  }
  return sum / SMOOTH_SIZE;
}

// Variance calculation function
float calcVariance(float* data, int size) {
  float sum = 0, sumSq = 0;
  for (int i = 0; i < size; i++) {
    sum += data[i];
    sumSq += data[i] * data[i];
  }
  float mean = sum / size;
  return (sumSq / size) - (mean * mean);
}

// Calibration function (analyze 15 blinks)
void calibrate() {
  Serial.println("Calibration starting...");
  Serial.println("Keep eyes OPEN and BLINK naturally until 15 blinks are detected...");

  float varianceSum = 0;         // Sum of variance for blinks
  int blinkCount = 0;            // Number of blinks detected
  unsigned long maxPause = 0;    // Longest pause between blinks
  unsigned long lastBlink = 0;   // Time of last blink
  bool firstBlink = true;
  int sampleCount = 0;

  unsigned long startTime = micros();
  while (blinkCount < 15) {
    while (micros() - startTime < (unsigned long)sampleCount * SAMPLE_DELAY);
    float raw = analogRead(eogPin) * (5.0 / 1023.0);
    window[sampleCount % WINDOW_SIZE] = smoothSignal(raw);

    if (sampleCount >= WINDOW_SIZE && sampleCount % WINDOW_SIZE == 0) {
      float currentVariance = calcVariance(window, WINDOW_SIZE);
      if (currentVariance > 0.02) { // Initial low threshold to catch blinks
        varianceSum += currentVariance;
        blinkCount++;
        unsigned long currentTime = millis();
        if (!firstBlink) {
          unsigned long pause = currentTime - lastBlink;
          if (pause > maxPause) maxPause = pause;
        }
        lastBlink = currentTime;
        firstBlink = false;
        Serial.print("Blink #"); Serial.print(blinkCount); 
        Serial.print(" Variance: "); Serial.println(currentVariance);
      }
    }
    sampleCount++;
    if (sampleCount >= 120 * SAMPLE_RATE) { // 2-minute timeout
      Serial.println("Timeout: Not enough blinks in 2 minutes.");
      break;
    }
  }

  // Set thresholds based on calibration
  if (blinkCount < 5) {
    Serial.println("Error: Too few blinks. Using defaults.");
    varianceThreshold = 0.01;     // Low default for blinks
    SLEEP_THRESHOLD = 15000;      // 15 seconds default
  } else {
    float avgBlinkVariance = varianceSum / blinkCount;
    varianceThreshold = avgBlinkVariance * 0.2; // 20% of avg blink variance
    SLEEP_THRESHOLD = maxPause * 2;             // 2x longest pause
    if (SLEEP_THRESHOLD < 10000) SLEEP_THRESHOLD = 10000; // Min 10s
    if (SLEEP_THRESHOLD > 30000) SLEEP_THRESHOLD = 30000; // Max 30s
  }

  Serial.print("Calibration Time (seconds): "); Serial.println(sampleCount / SAMPLE_RATE);
  Serial.print("Blinks Detected: "); Serial.println(blinkCount);
  Serial.print("Average Blink Variance: "); Serial.println(varianceSum / blinkCount);
  Serial.print("Variance Threshold: "); Serial.println(varianceThreshold);
  Serial.print("Longest Awake Pause: "); Serial.println(maxPause);
  Serial.print("Sleep Threshold: "); Serial.println(SLEEP_THRESHOLD);

  calibrated = true;
  lastBlinkTime = millis();
  Serial.println("Calibration complete! Starting continuous detection...");
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(speakerPin, OUTPUT);    // Set buzzer pin as output
  digitalWrite(ledPin, HIGH);     // Start awake
  digitalWrite(speakerPin, LOW);  // Ensure buzzer is off initially

  for (int i = 0; i < SMOOTH_SIZE; i++) {
    smoothBuffer[i] = 0;
  }
  for (int i = 0; i < WINDOW_SIZE; i++) {
    window[i] = 0;
  }

  calibrate(); // Run once at startup
}

void loop() {
  if (!calibrated) return;

  static unsigned long nextSampleTime = micros();
  unsigned long currentTime = micros();

  if (currentTime >= nextSampleTime) {
    float raw = analogRead(eogPin) * (5.0 / 1023.0);
    float smoothed = smoothSignal(raw);

    window[windowIndex] = smoothed;
    windowIndex = (windowIndex + 1) % WINDOW_SIZE;

    if (windowIndex == 0) {
      float currentVariance = calcVariance(window, WINDOW_SIZE);

      if (currentVariance > varianceThreshold) {
        lastBlinkTime = millis();
        if (!isAwake) {
          isAwake = true;
          digitalWrite(ledPin, HIGH);
          digitalWrite(speakerPin, LOW); // Turn off buzzer when waking up
          Serial.println("Awake");
        }
      }

      unsigned long timeSinceBlink = millis() - lastBlinkTime;
      if (isAwake && timeSinceBlink > SLEEP_THRESHOLD) {
        isAwake = false;
        digitalWrite(ledPin, LOW);
        digitalWrite(speakerPin, HIGH); // Turn on buzzer when asleep
        Serial.println("Asleep");
      }
    }

    nextSampleTime += SAMPLE_DELAY;
    if (nextSampleTime < currentTime) {
      nextSampleTime = currentTime + SAMPLE_DELAY;
    }
  }
}
