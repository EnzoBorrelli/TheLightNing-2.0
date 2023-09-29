class Sharp {
private:
  int _pin;

public:
  Sharp(int pin) {
    _pin = pin;

    pinMode(_pin, INPUT);
  }

  int GetValue() {
    int analogValue = analogRead(_pin);
    float voltage = analogValue / 4096. * 5;
    float resistencia = 2000 * voltage / (1 - voltage / 5);
    int sharp = pow(50 * 1e3 * pow(10, 0.7) / resistencia, (1 / 0.7));
    sharp = sharp / 10;

    return sharp;
  };
}