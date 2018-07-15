task2.md

To complete Task 2, I wrote a Python 2 script called "task2.py" that generates the necessary raw data, computes statistics based on the raw data, and then uses the Matplotlib library to create graphs of the statistics.

First the script performs some prepatory steps, including initializing two empty 4x6 matrices (using numpy), which will be used to store the data on average wait and turnaround times. It also creates two arrays storing the quantum lengths and dispatch costs specified in the assignment. Finally, it creates a Python regex object based on the known format of the output from the simulation program.

The script's algorithm is as follows:
    1. Using two for loops (one for the quantum lengths, the other for the dispatch costs), call the "./simgen" script to generate the specified number of tasks (1000) using the specified random-seed value (1127), and pipe the output of this script to the "./rrsim" script, passing the relevant args for quantum length and dispatch cost.
    2. After the "./rrsim" script has completed for the given quantum length and dispatch cost, the "task2.py" script analyzes the output generated by ./rrsim line by line, comparing each line against the regex object already created, looking for lines where a task has been completed. For each task that has been completed, its wait and turnaround time are added to a running sum of wait and turnaround times. A third variable counts the number of tasks that have been analyzed.
    3. Once every line output from ./rrsim has been read, the summed wait and turnaround times are divided by the number of tasks analyzed (1000), to compute the average wait and turnaround times. These averaged values are stored in the corresponding matrices for wait and turnaround times, indexed by the dispatch and quantum values used for that simulation. As a sanity check, the computed average values for each simulation are also printed to the console.
    4. Once all 24 simulations have been run and analyzed, task2.py plots the average wait and turnaround times based on the values stored in the 2 matrices. Plotting is done using the matplotlib.pyplot library. 

As a safety measure, the task2.py script checks at the end of each simulation that all 1000 tasks have been analyzed and accounted for in the sum of wait and turnaround times. If for some reason the number of tasks analyzed does not equal 1000, the script exits with an appropriate error message.