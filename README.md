# CursorHeatmap

## Useful links
- cJSON - https://github.com/DaveGamble/cJSON
- libattopng - https://github.com/misc0110/libattopng

## Usage
### Compile
`gcc -static main.c -o cursorHeatmap.exe`

### Configuration
After running `cursorHeatmap.exe` it will create a file named `data.json` in which config and data about mouse positions are saved.

#### Settings
- `polling_rate` how many microseconds does the program wait before it checks for mouse position again - default: 1000 (any less will cause way higher battery consumption)
- `debug` whether debug messages should be printed out - default: 0 (1 for true)
- `hidden` whether command prompt should be visible - default: 0 (1 to hide)
- `save_interval` how long in **minutes** does it take to automatically save positions from memory to file - default: 5 (10+ recommended)
- `rgb_background` RGB to the png file background - default: [0, 0, 0]
- `rgb_activity` RGB to the mouse trace - defautlt: [0, 255, 0]
