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
secret = np.random.random_sample(1)
print('My rank is ' + str(myRank) + ' and my secret is: ' + str(secret))

outFriend = (myRank+1) % comSize
inFriend = (myRank + comSize - 1) % comSize

comm.Isend(secret, dest = outFriend, tag=0)
friendSecret = comm.Irecv(secret, source= inFriend, tag=0)
friendSecret.Wait()
 
    
print('My rank is ' + str(myRank) + ' and I received the secret: ' + str(secret))
    
# To execute, run mpiexec -n 4 python secretCircleNB.py

