# Canteen Management System

A **C++ and Data Structures & Algorithms (DSA)** based project developed as part of our **Project-Based Learning (PBL)**.

## About the Project

The Canteen Management System aims to simplify the basic operations of a college canteen. It focuses on managing food items, customer orders, prices, and order history.

The core system is developed in C++, using data structures to organize and process information efficiently. The project is being developed in stages, with the possibility of integrating a web-based frontend and backend.

## Features

- Manage food items and prices
- Add and remove food items
- Place and manage customer orders
- Track food availability
- Search and sort food items
- Maintain customer order history
- Organize orders using data structures
- Save order records using file handling

## Technologies Used

### Core System
- C++
- Data Structures & Algorithms
- File Handling

### Planned Web Technologies
- HTML and CSS
- JavaScript and React.js
- Node.js and Express.js
- MongoDB

### Development Tools
- Git and GitHub
- Visual Studio Code
- macOS for local development and testing

## Project Structure

The project is organized into source files and header files to keep the implementation modular and maintainable.

- `src/` — C++ source files
- `include/` — Header files
- `main.cpp` — Application entry point, located within the source directory if applicable

## Running the Project on macOS

### Prerequisites
- macOS
- A C++ compiler such as `g++` or `clang++`
- Git

### Compile the Project

From the project root directory, run:

```bash
g++ src/main.cpp src/Food.cpp src/Order.cpp src/Customer.cpp src/Admin.cpp src/Cart.cpp -Iinclude -o canteen.out
```

### Run the Application

```bash
./canteen.out
```

**Note:** The compile command may need updating if source files or dependencies change.

## Team Members

- Manish
- Dhruv
- Bharat
- Lakshya

## Project Status

🚧 **Currently under development**

The C++ core system is being developed and tested locally on macOS. Additional improvements and web integration may be introduced in future stages.

---

Developed as part of our Project-Based Learning (PBL).