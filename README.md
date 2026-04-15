# CPS3525 Team Project - Maze Path Statistics Web CGI

---

## Deployment URL

Project homepage:  
`https://obi.kean.edu/~tangyua@kean.edu/CPS3525/index.html`

Team project page:  
`https://obi.kean.edu/~tangyua@kean.edu/CPS3525/project3.html`

CGI endpoint:
`https://obi.kean.edu/~tangyua@kean.edu/cgi-bin/CPS3525/maze.cgi`

---

## Project Overview
This project is a CPS3525 team web CGI application based on the Maze Game option.

The user uploads a 2-D maze text file. The maze uses:
- `B` for background
- `H` for path

The entrance is on the top row and the exit is on the bottom row.

The CGI program:
- reads the uploaded maze file
- validates the input
- finds all valid top-to-bottom paths
- displays step-by-step path coordinates
- counts the total number of valid paths
- optionally checks whether the user's guess is correct

---

## Repository Structure

The source files for this project are stored in the folder:

`team-project-CPS_3525/`

Main files include:
- `project3.html`
- `main.cpp`
- `function.cpp`
- `function.hpp`
- `get_validate_input.cpp`
- `get_validate_input.hpp`
- `web_types.hpp`
- `build.sh`
- `sample_maze.txt`

---

## File Responsibilities

### Front-end
- `project3.html`  
  HTML front-end form for file upload and user inputs.

### Back-end source files
- `main.cpp`  
  Main CGI program that receives the request, calls the validation module and the function module, and prints the final HTML result.

- `get_validate_input.cpp`  
  Receives and validates web inputs, parses multipart/form-data, checks maze file format, and handles validation errors.

- `get_validate_input.hpp`  
  Header for input receiving and validation functions.

- `function.cpp`  
  Implements maze business logic, including path search, path counting, guess checking, and result formatting.

- `function.hpp`  
  Header for maze path processing functions.

- `web_types.hpp`  
  Shared data structures used by the web input and maze processing modules.

### Build script
- `build.sh`  
  Compiles and links the C++ source files into a single CGI executable.

### Sample input
- `sample_maze.txt`  
  Example maze input file for testing.

---

## My Responsibilities
I was mainly responsible for the front-end design and web input validation part of this project, including:

- `project3.html`
- `get_validate_input.cpp`
- `get_validate_input.hpp`
- `web_types.hpp`

My work included:
- designing the HTML form and input layout
- handling file upload related inputs
- parsing `multipart/form-data`
- validating the maze input file
- checking rows, columns, and valid maze characters
- checking maze entrance and exit conditions
- handling default values and validation errors
- deploying and testing the project under my own `obi.kean.edu` account

---

## Teammate Responsibilities
My teammate mainly worked on the maze business logic and integration part, including:

- `function.cpp`
- `function.hpp`
- `main.cpp`
- `build.sh`

This part included:
- maze path searching
- path counting
- guess checking
- result generation
- CGI integration
- build script support

---

## Build and Run

On `obi.kean.edu`, go to the project source folder and run:

```bash
chmod 755 build.sh
./build.sh
