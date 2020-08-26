# PWM Servo demo

Demo for 2 servoes.

IO9 and IO10 is PWM out, duty can set by:

```c
#define SERVO0_DUTY_MAX (0.125)
#define SERVO0_DUTY_MIN (0.025)
#define SERVO1_DUTY_MAX (0.12)
#define SERVO1_DUTY_MIN (0.03)
```

Circuit connection:

```
+--------------+        +-----------+
|              |        |        VCC+----+5V~6V POWER
|          IO9 +--------+PWM        |
|              |        |   SERVO0  |
|              |        |           |
|              |        |        GND+----+GROUND
|              |        +-----------+
| K210         |
|              |        +-----------+
|              |        |        VCC+----+5V~6V POWER
|          IO10+--------+PWM        |
|              |        |   SERVO1  |
|              |        |           |
|              |        |        GND+----+GROUND
+--------------+        +-----------+

```