# GarageRemoteSimple

Build a garage Remote using Particle Photon with an HR SR04 to detect door status, notify via IFTT, Initiate commands from iOs Device

## What it is?

A ParticleIO's Photon microprocessor that connect to HR SR04 (Ultrasonic sensor) to determine if the garage door is opened or closed, it also connect to a 5VDC relay to trigger the garage door's open and close commands.

Additionally I can request this commands via IFTT, Siri via Shortcuts App, and alert using the Pushover's API (Push Notification)

## Equipment and Tools

- ParticleIO's Photon Microprocessor
- HR SR04
- HR SR04 3D printed Support
- 5 Vdc Relay
- Generic Garage Door Motor
- 5 Vdc Power Supply and Cable

## Setup IFTTT (Work in progress)

Still working on this steps...

### Trigger from IFTTT iOS App

### Trigger from Siri 

![Step 1](resources/IFTTT-WEBHOOK-01.png)

Use a IFTTT's webhook to acomplish this. 

## Welcome to the Particle IO project using VSCode Extension

### Particle Workbench Installation

Please visit [Particle Workbench](https://www.particle.io/workbench/) for further instrucctions.

### Folder Struccture

Every new Particle project is composed of 3 important elements that you'll see have been created in your project directory for GarageRemoteSimple.

#### ```/src``` folder:  
This is the source folder that contains the firmware files for your project. It should *not* be renamed. 
Anything that is in this folder when you compile your project will be sent to our compile service and compiled into a firmware binary for the Particle device that you have targeted.

If your application contains multiple files, they should all be included in the `src` folder. If your firmware depends on Particle libraries, those dependencies are specified in the `project.properties` file referenced below.

#### ```.ino``` file:
This file is the firmware that will run as the primary application on your Particle device. It contains a `setup()` and `loop()` function, and can be written in Wiring or C/C++. For more information about using the Particle firmware API to create firmware for your Particle device, refer to the [Firmware Reference](https://docs.particle.io/reference/firmware/) section of the Particle documentation.

#### ```project.properties``` file:  
This is the file that specifies the name and version number of the libraries that your project depends on. Dependencies are added automatically to your `project.properties` file when you add a library to a project using the `particle library add` command in the CLI or add a library in the Desktop IDE.

## Adding additional files to your project

#### Projects with multiple sources
If you would like add additional files to your application, they should be added to the `/src` folder. All files in the `/src` folder will be sent to the Particle Cloud to produce a compiled binary.

#### Projects with external libraries
If your project includes a library that has not been registered in the Particle libraries system, you should create a new folder named `/lib/<libraryname>/src` under `/<project dir>` and add the `.h`, `.cpp` & `library.properties` files for your library there. Read the [Firmware Libraries guide](https://docs.particle.io/guide/tools-and-features/libraries/) for more details on how to develop libraries. Note that all contents of the `/lib` folder and subfolders will also be sent to the Cloud for compilation.

## Compiling your project

When you're ready to compile your project, make sure you have the correct Particle device target selected and run `particle compile <platform>` in the CLI or click the Compile button in the Desktop IDE. The following files in your project folder will be sent to the compile service:

- Everything in the `/src` folder, including your `.ino` application file
- The `project.properties` file for your project
- Any libraries stored under `lib/<libraryname>/src`
