# MAX31855 library for ESP32 and ESP-IDF

This is an ESP-IDF library for reading temperatures from (K-type) thermocoupler with the MAX31855 chip.


## Get started
### Install the ESP-IDF SDK
[http://esp-idf.readthedocs.io/en/latest/](http://esp-idf.readthedocs.io/en/latest/)

Note:

This project is compatible only with ESP-IDF v4.x and the CMake-based build system.
Instructions here are given for the v4.x toolchain using `idf.py`.

### Build this repository standalone and run the example.

Try this first to make sure your hardware is supported, wired and configured properly.

1. Get this project: `git clone https://github.com/andreas-berg/max31855_esp32.git`

2. From its root run `idf.py menuconfig`

3. Set your SPI pin configurations (MISO,CLK,CS,BUS) under Components/MAX31855 Controller 

4. Save your project configuration.

5. `idf.py build`

6. `idf.py -p (YOUR PORT) flash`

7. `idf.py -p (YOUR PORT) monitor`, if everything works you should see the temperature readings logged.


### Install this project as a library submodule in your own project

It is recommended to install this repo as a submodule in your IDF project's git repo. The configuration system has been designed so that you do not need to copy or edit any files in this repo. 

From your project root (you can get the esp32 idf project template [here](https://github.com/espressif/esp-idf-template)):

1. `mkdir -p components`
2. `git submodule add https://github.com/andreas-berg/max31855_esp32.git components/max31855_esp32`
3. `git submodule update --init --recursive`
4. Edit your CMake to add this repo's components folder to the IDF components path.

    The examples below are taken from the ESP-IDF [blink](https://github.com/espressif/esp-idf/tree/master/examples/get-started/blink) example which you can copy and use as the basis for your own project.
The project root CMakeLists.txt file needs one line added, just before the project to add the extra components directory to the path like this:

    ```cmake
    #CMakeLists.txt
    cmake_minimum_required(VERSION 3.5)

    include($ENV{IDF_PATH}/tools/cmake/project.cmake)

    list(APPEND EXTRA_COMPONENT_DIRS components/max31855_esp32/components/max31855)

    project(blink)
    ```

    In the CMakeLists.txt file for your `/main` or for the component(s) using MAX31855 you need to add REQUIRES directives for this project's code to the `idf_component_register` function, it should look like this:


    ```cmake
    set (SOURCES main.c)

    idf_component_register(SRCS ${SOURCES}
        INCLUDE_DIRS .
        REQUIRES max31855)

    ```
5. Configure the module with `idf.py menuconfig` and then `idf.py -p (YOUR PORT) flash`.