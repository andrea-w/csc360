# task3.py

'''
    ALGORITHM
    1. Call ./simgen and pipe output to ./rrsim for each combo of quantum length and dispatch cost
        1.1.  For each simulation, search output to parse results - gather wait and turnaround times for each completed task, store in matrices (one for wait & another for turnaround). Each matrix will consist of 4 vectors (1 vector for each quantum length), and each vector will have 6 data points (for the 6 dispatch costs).
            a. For each completed task, add its wait & turnaround values to running sum of wait & turnaround values for each task. Once all tasks have been analyzed (count the number of tasks analyzed), save the sum to matrices.
            b. Check that for each simulation, the correct number of tasks have been completed & analyzed
    2. Once all simulations have been run and outputs parsed, divide all values in matrices by # of tasks (to compute average)
    3. Plot the data in the matrices
'''

import subprocess
import numpy
import re
import matplotlib.pyplot as plt

# args to ./simgen
num_tasks = "1000"
seed = "1127"

min_request = 99999999999.0
max_request = 0.0
total_request = 0.0
taskRegExObj = re.compile(" [0-9]+\.[0-9]{2}$")
# run ./simgen and find min and max cpu_requests of tasks generated
simgen = subprocess.Popen(["./simgen", num_tasks, seed], stdout=subprocess.PIPE)
for out in iter(simgen.stdout.readline, ''):
    r = taskRegExObj.search(out)
    if (r != None):
        request = float(r.group(0))
        total_request += request
        if (request < min_request):
            min_request = request
        if (request > max_request):
            max_request = request
avg_request = total_request / float(num_tasks)

# variables for quantum lengths and dispatch costs, as specified in assignment
quantum_len = ["50", "100", "250", "500", str(min_request), str(max_request), str(avg_request)]
dispatch_cost = ["0", "5", "10", "15", "20", "25"]

# initialize matrices for storing wait and turnaround times data
wait_matrix = numpy.empty((7,6))
turnaround_matrix = numpy.empty((7,6))

regExObj = re.compile(".* EXIT w=(.+) ta=(.+)")           # output in ./rrsim has been formatted this way when a task is completed

# https://docs.python.org/2/library/functions.html#enumerate
for q_index, q_len in enumerate(quantum_len):
    for d_index, d_cost in enumerate(dispatch_cost):

        '''
         got help with this from 
         https://blenderartists.org/t/calling-a-c-program-with-python/462789/3
         and
         https://docs.python.org/2/library/subprocess.html
         '''
        simgen = subprocess.Popen(["./simgen", num_tasks, seed], stdout=subprocess.PIPE)
        rrsim = subprocess.Popen(["./rrsim", "--quantum", q_len, "--dispatch", d_cost], stdin=simgen.stdout, stdout=subprocess.PIPE)

        count = 0                   # counts the number of tasks processed
        turnaround_sum = 0.0        # running total of turnaround time for each task processed
        wait_sum = 0.0              # running total of wait time for each task processed

        '''
        regex search documentation:
        https://docs.python.org/2/library/re.html
        and help from 
        https://www.tutorialspoint.com/python/python_reg_expressions.htm
        '''
        for line in rrsim.stdout:
            m = regExObj.search(line)           
            if (m != None):
                wait_sum += float(m.group(1))
                turnaround_sum += float(m.group(2))
                count += 1
        if (count != int(num_tasks)):
            sys.exit("Error. Count = " + str(count) + " ; should be " + str(num_tasks))


        wait_matrix[q_index][d_index] = wait_sum/count
        turnaround_matrix[q_index][d_index] = turnaround_sum/count

        print("wait[" + str(q_index) + "][" + str(d_index) + "]: " + str(wait_matrix[q_index][d_index]))
        print("turnaround[" + str(q_index) + "][" + str(d_index) + "]: " + str(turnaround_matrix[q_index][d_index]))

x = numpy.arange(0, 30, 5)

plt.subplot(211)
plt.plot(x, wait_matrix[0], marker="s", label="q=50")
plt.plot(x, wait_matrix[1], marker="o", label="q=100")
plt.plot(x, wait_matrix[2], marker="*", label="q=250")
plt.plot(x, wait_matrix[3], marker="x", label="q=500")
plt.plot(x, wait_matrix[4], marker="p", label="q=min="+str(min_request))
plt.plot(x, wait_matrix[5], marker="D", label="q=max="+str(max_request))
plt.plot(x, wait_matrix[6], marker="8", label="q=avg="+str(avg_request))
plt.title("Round Robin scheduler - # tasks: "+num_tasks+"; seed value: 1127")
plt.ylabel("Average waiting time")
plt.xlabel("Dispatch overhead")
plt.legend()
plt.grid(True)

plt.subplot(212)
plt.plot(x, turnaround_matrix[0], marker="s", label="q=50")
plt.plot(x, turnaround_matrix[1], marker="o", label="q=100")
plt.plot(x, turnaround_matrix[2], marker="*", label="q=250")
plt.plot(x, turnaround_matrix[3], marker="x", label="q=500")
plt.plot(x, turnaround_matrix[4], marker="p", label="q=min="+str(min_request))
plt.plot(x, turnaround_matrix[5], marker="D", label="q=max="+str(max_request))
plt.plot(x, turnaround_matrix[6], marker="8", label="q=avg="+str(avg_request))
plt.title("Round Robin scheduler - # tasks: "+num_tasks+"; seed value: 1127")
plt.ylabel("Average turnaround time")
plt.xlabel("Dispatch overhead")
plt.legend()
plt.grid(True)
plt.show()

