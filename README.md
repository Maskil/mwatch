# MWatch - a variant of Watchy

Apperances and codes are refactored to my personal tastes. BMA foot step counter is disabled. Here are the features:
- fetch weather info from OpenWeather.org per hour (you can change it in `mwatch.cpp`, variable `iclock`).
- you can send dumb [email](#email)
- it can receive a simple message via BLE (i.e nRF app on Android) 

## build

after [configuration](#configurations), do `sh build.sh {serial-port}`.

## extra Arduino library

you need [ESP Mail Client](https://www.arduino.cc/reference/en/libraries/esp-mail-client/) to compile.

## background

convert your wallpaper (200x200) into C array, then replace everything in `Image.h` to yours.


## configurations

everything you need to edit is in `settings.h`.

### weather

get weather info from [OpenWeather's API](https://openweathermap.org/current).

### email

send simple email which the content is fixed (or can be customized by HTTP requests, etc.).

### BLE

nothing much going on there. just put your service UUID and characteristic UUID in `BLE.cpp` and it should work.