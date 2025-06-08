# IOS - Project 2: Synchronization (Skibus)

## Project Description

This project is an implementation of a simulation based on "The Senate Bus problem" from Allen B. Downey's book "The Little Book of Semaphores". It simulates a skibus system and skiers waiting at bus stops.

The system consists of three types of processes:
1.  **Main Process:** Manages the startup of other processes and waits for their completion.
2.  **Skibus Process:** Simulates the bus driving between boarding stops and to the final drop-off stop. It handles skiers boarding at stops up to its capacity and dropping them off at the final stop.
3.  **Skier Process:** Simulates the behavior of an individual skier from "breakfast", arriving at a stop, waiting for the bus, boarding, riding, and getting off.

The project utilizes inter-process communication and synchronization techniques, specifically shared memory for shared variables and semaphores for synchronizing access to shared data and coordinating process activities.

## Building the Project

The project is built using the `make` utility. Ensure you have a C compiler (e.g., GCC) and `make` installed on your system.

Navigate to the directory where your `Makefile` and source files (`.c`, `.h`) are located, and run the command:

```bash
make
```
This command should compile the source code and create an executable file named `proj2`.

## Running the Program

The program is executed from the command line with four arguments:

```bash
./proj2 L Z K TL TB
```
Argument descriptions:
*   `L`: Number of skiers (`0 < L < 20000`)
*   `Z`: Number of boarding stops (`0 < Z <= 10`)
*   `K`: Skibus capacity (`10 <= K <= 100`)
*   `TL`: Maximum time (in microseconds) a skier waits/sleeps during the "breakfast" simulation (`0 <= TL <= 10000`)
*   `TB`: Maximum bus travel time (in microseconds) between two stops (`0 <= TB <= 1000`)

All input arguments must be positive integers within the specified ranges.

Example run:
```bash
./proj2 50 5 30 5000 500
```

## Error Handling

The program checks the format and range of input arguments. In case of invalid inputs, it will print an error message to standard error (`stderr`) and exit with a return code of 1.

If any system call related to processes (`fork`), shared memory, or semaphores fails during program execution, the program will also print an error message to `stderr`, attempt to release allocated resources, and exit with a return code of 1.

Successful completion of the simulation is indicated by a return code of 0 from the main process.

## Program Output

All program output is redirected to a file named `proj2.out`. Each action performed by a process is logged on a separate line. The format of each line is:

```
A: ProcessType id: action_details
```

*   `A`: Sequential action number, starting from 1. Actions are numbered globally across the entire simulation.
*   `ProcessType`: Type of process (`L` for skier, `BUS` for skibus).
*   `id`: Process identifier (Skier ID `idL` from 1 to `L`) or the stop identifier (`idZ` from 1 to `Z`) where the action occurs.
*   `action_details`: Description of the action performed.

Specific output messages:

**Skier (L idL):**
*   `A: L idL: started`
*   `A: L idL: arrived to idZ`
*   `A: L idL: boarding` (Must appear between `BUS: arrived to idZ` and `BUS: leaving idZ`)
*   `A: L idL: going to ski` (Must appear between `BUS: arrived to final` and `BUS: leaving final`)

**Skibus (BUS):**
*   `A: BUS: started`
*   `A: BUS: arrived to idZ`
*   `A: BUS: leaving idZ`
*   `A: BUS: arrived to final`
*   `A: BUS: leaving final`
*   `A: BUS: finish`

An example of the output is provided in the project assignment.

## Implementation Details

*   The project is implemented in C.
*   **Processes** (created using `fork`) are used, not threads.
*   **Shared memory** is used for the global action counter (for the `A` number in the output) and other shared variables necessary for synchronization.
*   **Semaphores** are the primary tool for synchronizing processes.
*   **Busy waiting is prohibited.** Waiting for skiers for the bus and waiting for the bus for skiers to board/alight must be implemented using semaphores that block the processes.
*   The "breakfast" (skier) and "travel" (bus) durations are simulated using the `usleep` call with a random time within the given maximum range (`TL` or `TB` respectively).
*   Each skier randomly chooses their boarding stop (from 1 to `Z`) upon startup.
*   Skiers can board/alight in any order (not necessarily `FIFO`), provided the conditions are met (bus at the stop, available capacity).
*   The bus continues to the next round of stops 1 to `Z` after serving the final stop if there are still any skiers waiting (i.e., they haven't arrived at a stop yet, are waiting at a stop, or are currently on the bus). The bus finishes (`BUS: finish`) only when no skiers are waiting at stops and all skiers who were on the bus have alighted at the final stop and finished.

## Resources

*   Project assignment "IOS – projekt 2 (synchronizace)"
*   Allen B. Downey: The Little Book of Semaphores