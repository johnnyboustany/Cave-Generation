# Cave Generation

This project is a real-time OpenGL scene viewer designed to showcase an interactive cave scene composed of an impressive 16,050 cubes. It also includes an efficient implementation of shadow mapping for all shadow-casting objects, with optimized edge smoothness achieved through Percentage-Closer Filtering (PCF), leading to a 75% improvement in visual quality.

<p align="center">
    <img src="./assets/cavebanner.png" alt="" width="1000">
</p>

## Table of Contents
* [Technologies Used](#technologies-used)
* [General Info](#general-info)
* [Features](#features)
* [Usage Instructions](#usage-instructions)
* [Project Status](#project-status)
* [Conclusion](#conclusion)
* [Contributions](#contributions)
<!-- * [License](#license) -->

## Technologies Used
C++, OpenGL

## General Info

The Real-Time OpenGL Cave Scene Viewer is a visually stunning application that allows users to explore a highly interactive cave environment. It showcases the capabilities of real-time rendering and advanced techniques like shadow mapping and edge smoothing using PCF to achieve a more realistic and immersive experience.

## Features

- Real-time rendering of a cave scene with 16,050 cubes.
- Interactive camera controls for user exploration.
- Efficient shadow mapping for all shadow-casting objects.
- 75% optimized edge smoothness through PCF.
- Adjustable settings for scene and rendering customization.

<p align="center">
    <img src="./assets/shadowmapping1.png" height=400 alt="">
    <br>
    above: shows the shadow mapping outside the cave.
</p>

<br>

<p align="center">
    <img src="./assets/shadowmapping2.png" height=400 alt="">
    <br>
    above: shows the shadow mapping inside the cave.
</p>

## Usage Instructions

### To run the Cave Scene Viewer, follow these steps:

1. Clone the project repository from GitHub:

        git clone https://github.com/johnnyboustany/cave-generation.git

2. Navigate to the project directory:

       cd cave-generation

4. Install the necessary dependencies (OpenGL, GLFW, etc.).
    
5. Build the project using your preferred build system (e.g., CMake, Make, etc.)
6. Run the code, which will make the GUI with the Cave Scene Viewer pop up

## Project Status
Project is: Complete (as of December 2022)

## Contributions

 I specifically worked on:
- the shadow mapping
- pcf

My group members: @coltonrusch, @mohammedakel, @JAnagonye, @glet2024, @mstephe7
