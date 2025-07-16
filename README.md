# Alicia launcher
Simple Alicia game series launcher built for Windows.
![image](https://github.com/rgnter/alicia_launcher/assets/32541639/28aba830-fe81-41f6-a948-2cfdd03867ad)

## Bridge
Bridge is a simple program which is supposed to be launched by Windows through custom launch protocol.
It spawns and configures the CLI launcher through program arguments. 

The URI parsed and expected by the bridge program looks like this:
```
soa://GAME_ID?username=USER_NAME&token=USER_TOKEN
```

Registration of the launch protocol should be handled by an installer.

## Building on Linux using mingw-w64-gcc
```bash
$ cmake -DCMAKE_BUILD_TYPE=Debug --toolchain toolchains/linux-mingw.cmake . -Bbuild
$ cd build/
$ cmake --build .
```