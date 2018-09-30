Changelog for Kendryte K210
======

## 0.1.0

Kendryte K210 first SDK with FreeRTOS, have fun. 

## 0.2.0

- Major changes
  - Rework trap handling 
  - New functions to enable spi0 and dvp pin 
  - New functions to select IO power mode
- Breaking changes
  - Modify struct enum union format
- Non-breaking bug fixes
  - Fix spi lcd unwork issues
  - Fix dual core startup issues
  - Use "__global_pointer$" instead of "_gp"
  
## 0.3.0

- Major change
  - Modify AES、FFT、SHA、I2C、SPI、WDT、SPI driver
- Breaking changes
  - Modify struct enum union format
- Non-breaking bug fixes
  - Fix out of memory issues
  - Fix lcd unused issues
  
