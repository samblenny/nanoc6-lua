# NanoC6 Lua

Lua interpreter for the M5Stack NanoC6 with ESP-IDF


## New Project Repo Setup Procedure

Create project, configure git remote, set target
```
cd ~/esp/esp-idf
source export.sh
cd ~/esp
idf.py create-project nanoc6-lua
cd nanoc6-lua
git init
git add origin https://github.com/samblenny/nanoc6-lua.git
# copy .gitignore and LICENSE
git add .
git commit -m 'initial commit'
git branch --set-upstream-to=origin/main main
git push
idf.py set-target esp32c6
```

Configure build with menuconfig
```
idf.py menuconfig
```
Then:
1. Build type > Enable reproducible build > YES
2. Bootloader config > Log > Bootloader log verbosity > Error
3. Serial flasher config
   - Flash size > 4 MB
   - Detect flash size when flashing bootloader > YES
4. Compiler options
   - Optimization level > Optimize for size
   - Assertion level > Disabled
5. Component config
   - ESP System Settings > CPU frequency > 80 MHz
   - Log > Log Level > Default log verbosity > Error


## Build and Run

```
idf.py flash monitor
# Ctrl-] to exit
```
