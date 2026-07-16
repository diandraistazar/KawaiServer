# KawaiServer
## Introduction
### Why I rewrote KawaiServer from C++ to Java
KawaiServer previously was written on C++ for performance and linux only. But after I switched to Windows, I couldn't run my computer as server with KawaiServer. To fix that, then I learned Java and completely rewrote the KawaiServer C++ source code into Java. It took for weeks to complete. Even so, the result is very satisfying and KawaiServer has the ability to be executed everywhere.

## Features
KawaiServer has few features that I have implemented, currently only:
- Multi-platform compatibility (Windows, Linux, MacOS, etc).
- GET method handling support.
- URL parameter handling support.
- Automatically find either ```index.html``` or ```index.php``` in the requested path only if the requested path is directory. If they aren't exists, then find them in the root directory.
- URL Special character handling support (e.g %20 for whitespaces).

## How to install
Before you can install or run KawaiServer, you need some dependencies, which are:
- OpenJDK 8
- PHP (Optional)

After install them, do these below steps:
```
git clone https://github.com/diandraistazar/KawaiServer.git # Clone this repository into your local machine
cd .\KawaiServer

# Run one of below commands according to what OS do you use
.\build.ps1 compile # for Windows
./build.sh compile # for Linux
```

## How to run
After install or rather compiling, then run:
```
java -jar KawaiServer.jar
```

## How to uninstall
To uninstall KawaiServer, just run:
```
# Run one of below commands according to what OS do you use
.\build.ps1 clean # for Windows
./build.sh clean # for Linux
```
