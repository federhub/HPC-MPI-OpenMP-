# -*- coding: utf-8 -*-
"""
Introduction to MPI in python.
Uses the library mpi4py.
@author: famato1
"""
import numpy as np
from mpi4py import MPI

comm = MPI.COMM_WORLD

comSize = comm.Get_size()
myRank = comm.Get_rank()

# They all pick a number
myNumber  = np.random.random_sample(1)
print('Rank: ' + str(myRank) + ' Number: ' + str(myNumber))


sum_ = np.zeros(1)
avg_ = np.zeros(1)

comm.Allreduce(myNumber, sum_, op=MPI.SUM)
avg_ = sum_/comSize

# They compute the difference between their number and
# the average

diff = myNumber - avg_
print('Rank: ' + str(myRank) + ' Diff is: ' + str(diff))

# Rank 0 looks for the largest difference and print it
max_ = np.zeros(1)
comm.Reduce(diff, max_, op=MPI.MAX, root=0)
if myRank ==0:
    print('Rank: ' + str(myRank) + ' Max diff is:: ' + str(max_))
      
# To execute, run mpiexec -n 4 python messyFixed2.py

