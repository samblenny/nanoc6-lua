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


## Setting Up Unit Tests

To get unit tests working, you need to combine some of the information from
Espressif's
[Unit Testing](https://docs.espressif.com/projects/esp-idf/en/v6.0/esp32c6/api-guides/unit-tests.html)
documentation page with the methods demonstrated in the
[examples/system\_unit\_test](https://github.com/espressif/esp-idf/tree/master/examples/system/unit_test)
example project in the esp-idf repo on GitHub.

Key Points:
1. Code to be tested needs to be a component
2. Unit tests go in `components/whatever/tests` in your project repo
3. You also need a test runner app in /test next to /main at the top level of
   your project. The test app runner should follow the `unit_test` example for
   CMakeLists.txt setup and code to invoke the Unity test library.
4. To build and run the test app:
   ```
   cd $REPO_ROOT_DIR/tests     # IMPORTANT: cd to tests before running idf.py!
   idf.py set-target esp32c6   # menuconfig won't work right without this
   idf.py menuconfig           # set STDIO to use USB serial
   idf.py flash monitor        # flash and run as usual
   ```
   Test runner output will print on the serial console.


## License and Copyright Notices

Copyright (c) 2026 Sam Blenny
<br>This nanoc6-lua project is licensed under the MIT license.

Copyright © 1994–2025 Lua.org, PUC-Rio.
<br>Lua is licensed under the MIT license.
See [components/lua-5.4.8/doc/readme.html](components/lua-5.4.8/doc/readme.html).
