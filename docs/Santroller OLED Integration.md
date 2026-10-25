To integrate an **SSD1306 128x32 I2C OLED display** as a custom device in Santroller, you can mirror the pattern used by WS2812 NeoPixels and other peripheral output devices 1, 2\. The implementation spans five core layers across the repository 3, 4:

### 1\. Protobuf Configuration Schema (proto/)

Santroller uses Protobuf definitions to structure configuration data passed between the firmware and the Santroller Configurator GUI 2\.

* **Add display settings**: Update or add a message in the .proto files under proto/ (e.g., devices.proto) 2, 3\.  
* **Define parameters**:  
* enabled (boolean flag)  
* i2c\_bus (e.g., i2c0 or i2c1 on RP2040) 1  
* sda\_pin and scl\_pin (e.g., GP4/GP5) 1, 2  
* i2c\_address (default 0x3C for SSD1306)  
* screen\_width (128) and screen\_height (32)

### 2\. Device Class Abstraction (include/ & src/)

In Santroller, output peripherals inherit from a base device interface 1, 2\. You will create a dedicated OLEDDisplay class under include/devices/oled\_display.hpp and src/devices/oled\_display.cpp 3\.

* **Class Structure**:  
* class OLEDDisplay : public Device {  
* public:  
*     bool init(const OledConfig& config);  
*     void update() override;  
*     void draw\_status(ConnectionStatus status, uint8\_t player\_number);  
*     void flush\_framebuffer();  
* private:  
*     uint8\_t framebuffer\[128 \* 32 / 8\]; // 512 bytes for 128x32 monochrome display  
*     i2c\_inst\_t\* i2c\_hardware;  
*     uint8\_t i2c\_addr;  
* };

### 3\. RP2040 Hardware I2C & SSD1306 Initialization

On the Raspberry Pi Pico, hardware communication utilizes the Pico SDK hardware\_i2c library 1, 2\.

* **I2C Initialization**:  
* Call i2c\_init(i2c\_hardware, baudrate) (typically 400kHz fast mode).  
* Configure GPIO pins with gpio\_set\_function(sda\_pin, GPIO\_FUNC\_I2C) and gpio\_set\_function(scl\_pin, GPIO\_FUNC\_I2C).  
* Enable internal pull-ups using gpio\_pull\_up().  
* **SSD1306 Command Sequence**:  
* Send initialization command stream via i2c\_write\_blocking():  
* Display OFF (0xAE)  
* Set multiplex ratio to 31 for 32-line height (0xA8, 0x1F)  
* Set display offset & start line (0xD3, 0x00, 0x40)  
* Enable charge pump regulator (0x8D, 0x14)  
* Set memory addressing mode to horizontal (0x20, 0x00)  
* Display ON (0xAF)  
* **Buffer Flush**:  
* Send control byte 0x40 (data mode) followed by the 512-byte framebuffer array to render text and graphics.

### 4\. Event Loop Integration & Status UI

Santroller's central update loop drives reactive output devices based on controller state changes 1, 2\.

* **State Triggers**: Connect the OLED display class to system status notifications 1, 2:  
* **Connection Status**: Display state strings such as "Connecting...", "USB Connected", or "Console Mode" 1, 2\.  
* **Player Index**: Render "Player 1" through "Player 4" (or P1–P4 icons) corresponding to the assigned console LED index 1, 2\.  
* **Profile / Mode**: Display active layout info (e.g., Guitar, Drums, or Gamepad profile).  
* **Frame Rate Management**: Update text rendering in draw\_status() only when state changes occur, then flush the buffer periodically inside update() to keep I2C bus traffic low.

### 5\. CMake Build System (CMakeLists.txt)

Ensure the new device sources compile for RP2040 target boards 1, 3\.

* Add src/devices/oled\_display.cpp to the main executable source list in CMakeLists.txt 3\.  
* Link hardware\_i2c from the Pico SDK dependencies 1\.

💡 **Next Step**: Would you like to draft the C++ header and implementation code for the OLEDDisplay class and SSD1306 initialization sequence?  
