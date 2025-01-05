# Conway's Game of Life

This repository contains multiple implementations of Conway's game of life developed for the CS 481 High Performance
Computing course. Each version (homework submission) is stored as a standalone module in its respective folder.
Additionally, a final version of the utility files can be found in the `util`folder. The `GameOfLife` folder holds files
used for development.

The project was structured into a series of 5 homeworks. Reports for each can be found in the `reports` folder.

## Homework 1: Basic Implementation

The first assigned homework simply asked for any verified implementation of Conway's game of life as a baseline. I
implemented it using a threadpool and a dedicated class for board tracking. Unfortunately, this version underperformed,
doing worse than the teacher's single threaded implementation.
Source code: `homework1Submission`

- The report for homework 1 is at `reports/Homework1-report.pdf`
- The files for homework 1 are stored in the `homework1Submission/` directory.
- Build instructions are at the top of `homework1Submission/GameOfLife.cpp`

## Homework 2: Server Testing

The second homework required testing our implementation's performance on the Alabama supercomputing cluster that we
would continue to use throughout the project. During this update I shifted the implementation to a single threaded
approach and tested its performance. Results can be found in `reports/Homework2-report.pdf`

- The report for homework 2 is at `reports/Homework2-report.pdf`
- The files for homework 2 are stored in the `homework2Submission/` directory.
- Test scripts can be found in `homework2Submission/test-scripts/`
- Output files for homework 2 are stored in `homework2Submission/test-results/`
- Build instructions are at the top of `homework2Submission/GameOfLife.cpp`

# Homework 3: Multithreading with OpenMP

Homework 3 asked us to implement Conway's Game of Life using the OpenMP multithreading library.
In this implementation, I made significant changes to the program's design. The structrue here is then repeated in the
following assignments. After assignment 2, the teacher gave us reference code for the single threaded instance of
Conway's Game of Life. As mentioned above, this single threaded code outperformed my threadpool approach. I realized
that my code was overcomplicated, and, through further testing, I realized that simplifying to a few files and removing
classes altogether would significantly improve performance. In this version most code is consolidated to the standalone
file. Additionally, the logic to evaluate cell changes is simplified. These changes result in improved performance.

- The report for homework 3 is at `reports/Homework3-report.pdf`
- The files for homework 3 are stored in the `homework3Submission/` directory.
- Test scripts can be found in `homework3Submission/test-scripts/`
- Output files for homework 3 are stored in `homework3Submission/test-results/`
- Build instructions are at the top of `homework3Submission/GameOfLifeStandalone.cpp`

# Homework 4: Multiprocessing with MPI

Homework 4 asked us to implement Conway's Game of Life using the Open MPI multiprocessing library. This implementation
uses the structure from homework 3 as a baseline, but adds significant changes to accommodate the MPI framework. This
implementation outperforms all previous implementations. Results comparing performance can be found in the report.

- The report for homework 4 is at `reports/Homework4-report.pdf`
- The files for homework 4 are stored in the `homework4Submission/` directory.
- Test scripts can be found in `homework4Submission/`
- Output files for homework 4 are stored in `homework4Submission/test-results/`
- Build instructions are at the top of `homework4Submission/GameOfLifeStandalone.cpp`

# Homework 5: Massive Multithreading with Cuda

For this assignment, we implemented Conway's Game of Life using massive multithreading with CUDA. This version of the
program outperformed all previous versions and reports of this performance can be found in the `reports` folder.

- The report for homework 5 is at `reports/Homework5-report.pdf`
- The files for homework 5 are stored in the `homework5Submission/` directory.
- Test scripts can be found in `homework5Submission/`
- Output files for homework 5 are stored in `homework5Submission/test-results/`
- Build instructions are at the top of `homework5Submission/GameOfLifeStandalone.cpp`
