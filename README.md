# Amusement-park-management-system

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Running-application](#Running-application)
- [Running-Tests](#Running-Tests)
- [Usage](#usage)
- [File-Structure](#File-Structure)

## Overview
This project is an Amusement Park Management System designed to streamline ticket booking, schedule management, and customer interaction. It features a robust search system based on inverted indexing, developed in C++, and integrates seamlessly with a web interface built using Flask.


## Features
- **Search Functionality**: High-performance full-text search using an inverted index on the C++ server.
- **Attractions surfing**: Users can browse available attractions and planned sessions.
- **Dynamic Update**: Scheduler for tracking file changes and reindexing data automatically.
- **Stress Testing**: The project includes tools to test server performance under high load.


## Requirements
- Server Requirements:
  - C++ compiler with C++17 or higher support.
  - asio and nlohmann/json libraries.
- Web Requirements:
  - Python 3.x
  - Python libraries: flask, socket, jsonify, sqlalchemy, pytest(for testing)
- Testing Tools:
  - pytest for Python.
  - A stress-testing script (test_stress_server.py).

## Installation
1. Clone the Repository:
```sh
git clone https://github.com/Neamen1/Amusement-park-management-system.git
cd Amusement-park-management-system
```

2. Set up the C++ Server:
  1. Navigate to the server/ directory
  2. Compile the C++ server using a compatible compiler:
  ```sh
  g++ -std=c++17 -lpq -pthread -o amusement_server server.cpp threadpool.cpp InvertedIndex.cpp
  ```
  3. Start the server:  ./amusement_server

3. Set up the PSQL database server from DB backup file
   
4. Set up the Flask Web Application
- Configure app/config.py to your needs, set conneciton data for database
- run the web application by launching runmyapp.py with Python
- Access the application at http://localhost:5000.

## Running-application

To run both the server and web interface:
1. Start the compiled C++ 
2. Access the Application:
Open your web browser and go to http://localhost:5000.

## Running-Tests
To run the tests, run 'pytest' inside app/tests folder

## Usage
- Admin Panel: Manage sessions, attractions.
- User Interface: Search attractions, explore sessions.
- Server: Build inverted index, search phrases, monitor files

## File-Structure
- Inv Index module/: Contains the C++ server source code files.
  - main.cpp: nodule input file
  - server.cpp: Core server logic and client request handling.
  - InvertedIndex.cpp: Implementation of the inverted index structure.
  - threadpool.cpp: Custom thread pool for concurrent task execution.
- app/: Contains the Flask web application.
  - \_\_init\_\_.py: load config and blueprints, create flask app and database instance
  - models.py: database model for sqlalchemy
  - config.py: configure flask app varialbes
  - routes/: routes backend coed
  - templates/: HTML templates for web pages.
  - static/: Static files like CSS, images and JavaScript.
  - tests/: Testing scripts for server and web components.


