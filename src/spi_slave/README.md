# Hardware diagram

```ditaa {cmd=true args=["-E"]}

                 ^
                 |
                +++
                | |2KΩ Pull-up
                +++
                 |
+------------+   |   +------------+
|            |   +---+INT         |
|            |   |   |            |
|          CS+---+-->+CS          |
|            |       |            |
|  SPI    CLK+------>+CLK  SPI    |
|  MASTER    |       |     SLAVE  |
|         DAT+-------+DAT         |
|            |       |            |
|         INT+<------+DRY         |
|            |       |            |
|            |       |            |
+------------+       +------------+

```
