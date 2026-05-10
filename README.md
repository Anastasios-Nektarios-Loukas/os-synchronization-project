# Multithreaded Pizza Delivery System (pthreads)

## Project Overview
This project is a concurrent simulation of a pizza restaurant's workflow, developed for the **Operating Systems** course at the **Athens University of Economics and Business (AUEB)**. The application uses **POSIX threads (pthreads)** to simulate multiple customers placing orders simultaneously, while managing a finite pool of resources.

## Features & Synchronization
The system simulates the entire lifecycle of an order:
1. **Order Reception:** Customers wait for an available operator (telephonist).
2. **Preparation:** Once an order is placed, a cook is assigned to prepare the pizzas.
3. **Baking:** Pizzas are placed in a limited number of ovens.
4. **Delivery:** After baking, a driver delivers the order and returns to the shop.

### Synchronization Mechanisms
To prevent race conditions and manage shared resources, the project implements:
- **Mutexes:** `OrderMutex`, `CookMutex`, `OvenMutex`, and `DeliveryMutex` to protect shared counters and resource states.
- **Condition Variables:** Used to block threads when resources (like ovens or drivers) are exhausted and signal them when they become free.

## Technical Implementation
- **Language:** C.
- **Libraries:** `<pthread.h>`, `<time.h>`, `<unistd.h>`.
- **Statistics:** The program outputs performance metrics including:
    - Total revenue and failed orders.
    - Average and maximum waiting time for customers.
    - Average and maximum cooling time of pizzas.

## How to Run
The program requires two arguments: the number of customers and a random seed.

1. **Compile:**
   ```bash
   gcc -O3 -pthread pizza.c -o pizza
