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


## Add Lua Component

```
cd ~/esp/nanoc6-lua
idf.py create-component -C components lua-5.4.8
cd components
tar xzf ~/Downloads/lua-5.4.8.tar.gz
cd lua-5.4.8/src
tar xzf ~/Downloads/one.tar.gz  # Lua 5.4 onefile tarball
```

After this, edit components/lua-5.4.8/CMakeLists.txt to include the right
files from lua-5.4.8/src/.


## License and Copyright Notices

Copyright (c) 2026 Sam Blenny
<br>This nanoc6-lua project is licensed under the MIT license.

Copyright © 1994–2025 Lua.org, PUC-Rio.
<br>Lua is licensed under the MIT license.
See [components/lua-5.4.8/doc/readme.html](components/lua-5.4.8/doc/readme.html).
