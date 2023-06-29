# Bare-Metal RTOS

This repository contains the source code for the **Bare-Metal RTOS** development project.



## Introduction

* Developed a real‐time operating system (RTOS) on the bare‐metal STM32F407‐Discovery board

### 1. RTOS using STM32CubeIDE

* Source code:  [./STM32CubeIDE](./STM32CubeIDE)
* Used **STM32CubeIDE** for software development

### 2. RTOS without using IDE (Bare-Metal)

* Source code: [./bare-metal](./bare-metal)
* Used **Vim** editor for software development, **GNU Arm Embedded Toolchain** for cross‐compiling, and **OpenOCD** with **GDB** client to load and debug the project on the target board



## Demonstration

Following is the demonstration of the test application for the 'Bare-Metal RTOS' project in action. 

<iframe width="560" height="315" src="https://www.youtube.com/embed/MYxrrz4UWkc" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" allowfullscreen></iframe>

- Scheduling Algorithm: Round-Robin 
- 4 tasks blinking each LED at a defined frequency:
  - Green: 1000 ms 
  - Orange: 500 ms 
  - Blue: 250 ms
  - Red: 125 ms
