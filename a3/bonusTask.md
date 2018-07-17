bonusTask.md

For the bonus task, I chose to explore the possible optimization of quantum lengths (in a fantasy world where the necessary
 compute time of each task is known in advance).

Before running the task2.py script, my script for the bonus task first searches through all the tasks that will be submitted to
 the simulator, and computes the minimum and maximum CPU requests, as well as the average CPU request. These 3 durations are
 added to the array of quantum lengths, and then the script executes as it did in task2.py.

I ran the task3.py script (the bonus task) for 1000 tasks and again for 2000 tasks, in order to examine the effects of the
 quantity of tasks on the average waiting and turnaround times, given the same dispatch overheads and quantum lengths. The
 output for these 2 runs of the task3.py script are included in "graphs_task3.pdf".

From the output of task 3, we can observe the same trend seen in the output of task2.py -- the average waiting and turnaround
 times decrease as the quantum length increases. The difference in average times becomes more significant with increasing 
 dispatch overheads. The output of task 3 also shows that the average waiting and turnaround times are minimized when the
 quantum length is equal to the maximum CPU request of all tasks submitted to the simulator. This means that it is more
 time-efficient to allow each task to run to completion, rather than restricting each task run to a shorter quantum length. In
 other words, executing tasks on a first-come-first-serve basis is more efficient than round robin scheduling (because fewer
 dispatches are required). 

Comparison of the graphs for 1000 tasks and 2000 tasks shows that the average times are approximately double for 2000 tasks
 than for 1000 tasks, given the same quantum length and dispatch cost. This means that average wait times increase linearly and
 directly proportionally to the number of tasks submitted to the simulator.

 To run the task3.py script, from the terminal, type
 python task3.py

 Note that the "task3.py" script is compatible with Python 2.7, but works only inconsistently with Python3.