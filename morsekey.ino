unsigned long downAt = 0;
unsigned long longPressedAt = 0;
unsigned long morsedAt = 0;

unsigned long debounceTimeout = 20;
unsigned long longPressTimeout = 500;
unsigned long resetTimeout = 3000;

int inputPin = A0;
int outputPin = 10;

class Morse
{
public:
  void add(bool value)
  {
    buffer[end] = value;
    end = (end + 1) % maxSize;

    if (size < maxSize)
    {
      size++;
    }
    else
    {
      start = (start + 1) % maxSize;
    }
  }

  bool check()
  {
    if (size != maxSize)
    {
      return false;
    }

    for (int i = 0; i < size; ++i)
    {
      int index = (start + i) % maxSize;
      if (buffer[index] != code[i])
      {
        return false;
      }
    }
    return true;
  }

  void reset()
  {
    start = 0;
    end = 0;
    size = 0;
    Serial.println("reset");
  }

  void print()
  {
    for (int i = 0; i < size; ++i)
    {
      int index = (start + i) % maxSize;
      Serial.print(buffer[index]);
    }
    Serial.println();
  }

private:
  static const int maxSize = 5;
  bool buffer[maxSize];
  bool code[maxSize] = {0, 0, 0, 0, 0}; // <- change this
  int start = 0;
  int end = 0;
  int size = 0;
};

class Debouncer
{
public:
  Debouncer(int64_t _timeout) : timeout(_timeout) {}
  bool update(bool value)
  {
    if (first)
    {
      state = value;
      previousValue = value;
      first = false;
    }

    if (value != previousValue)
    {
      startedAt = millis();
    }

    if (expired(startedAt, timeout))
    {
      state = value;
    }

    previousValue = value;
    return state;
  }

  static bool expired(unsigned long time, unsigned long _timeout)
  {
    return time && (millis() - time) >= _timeout;
  }

private:
  bool first = true;

  int64_t timeout;
  int64_t startedAt = 0;

  bool state;
  bool previousValue;
};

Debouncer debouncer(debounceTimeout);
Morse morse;

void setup()
{
  Serial.begin(115200);
  pinMode(inputPin, INPUT);
  pinMode(outputPin, OUTPUT);
}

void loop()
{
  bool state = debouncer.update(analogRead(inputPin) > 20);

  if (!downAt && state) // down
  {
    downAt = millis();
  }
  else if (downAt && !state)
  {
    downAt = 0;
    morsedAt = millis();

    if (longPressedAt) // long up
    {
      // Serial.println("long up");
      morse.add(true);
      morse.print();
      longPressedAt = 0;
    }
    else // up
    {
      morse.add(false);
      morse.print();
      // Serial.println("up");
    }

    if (morse.check())
    {
      Serial.println("open");
      digitalWrite(outputPin, HIGH);
      delay(1000);
      digitalWrite(outputPin, LOW);
    }
  }

  if (Debouncer::expired(downAt, longPressTimeout) && !longPressedAt) // long down
  {
    longPressedAt = millis();
  }

  if (Debouncer::expired(morsedAt, resetTimeout))
  {
    morse.reset();
    morse.print();
    morsedAt = 0;
  }
}
