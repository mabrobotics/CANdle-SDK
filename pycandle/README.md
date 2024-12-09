# MD80 Python driver
This Python package can be used to configure and control MD80-based actuators using MAB's communication dongle (CANdle or CANdle HAT). The package is a wrapper around the C++ library - with simple python examples there won’t be any noticeable performance decrease compared to plain C++, however for real-time applications we advise using C++ libraries directly. 

The original C++ libraries and python examples are located in the [candle](https://github.com/mabrobotics/candle) repository. Please make sure you read the [MD80 x CANdle manual](https://www.mabrobotics.pl/servos/manual) before you start playing with the examples.

# Getting started

Assuming you've already read the manual from previous section and performed the necessary setup steps now's the time to install pyCandle package:

```python3 -m pip install pyCandleMAB```

or when you're working with Raspberry PI:

```sudo python3 -m pip install pyCandleMAB```

when the package is installed without errors connect CANdle device to your PC and run the first example:

```python3 ./example1.py```

when a signle actuator is connected to the dongle the console output should look simmilar to:

```[CANDLE] CANdle library version: v3.0
[CANDLE] Creating CANdle object.
[CANDLE] Reset successfull!
[CANDLE] CANdle ready.
[CANDLE] Starting pinging drives...
[CANDLE] Found drives.
[CANDLE] 1: ID = 85 (0x55)
[CANDLE] LEDs blining at ID = 85
EXIT SUCCESS
```
For more examples visit: https://github.com/mabrobotics/candle/tree/main/examples_python
