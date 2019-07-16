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
friendSecret = -1
print('My rank is ' + str(myRank) + ' and my secret is: ' + str(secret))



if myRank == 0:
    comm.send(secret, dest = 1, tag=0)
elif myRank != (comSize-1):
    friendSecret = comm.recv(source= myRank-1, tag=0)
    comm.send(secret, dest = myRank+1, tag=0)
else:
    friendSecret = comm.recv( source= myRank-1, tag=0)

if( myRank != 0 ):
    print('My rank is ' + str(myRank) + ' and I received the secret: ' + str(friendSecret))
    
# To execute, run mpiexec -n 4 python secretChain.py

