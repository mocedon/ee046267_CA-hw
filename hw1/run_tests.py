import glob
import os
import sys
import subprocess as sp
import pandas as pd

args = sys.argv
print(args)
#print("hello")
#files = glob.glob('input_examples/*', recursive=True)
#print(files)

test = pd.read_csv("tests.csv", header=None)
#print(test)
ret_ = 0
for i, t in test.iterrows():
    #print(t)
    f = open("./tmp.txt", 'w')
    arg = "{} {}".format(args[1], t[0])
    ret = sp.call(arg, stdout=f, stderr=f, shell=False)
    f.close()
    print("Test : {}".format(t[0]))
    if ret == 0:
        res_ = open("./tmp.txt", 'r')
        ref_ = open(t[1], 'r')
        res = res_.readlines()
        ref = ref_.readlines()
        res_.close()
        ref_.close()
        for i in range(len(res)):
            if res[i].strip() != ref[i].strip():
                print("    line:{:2d} mismatch : {:40} | {:40}".format(i, res[i].strip(), ref[i].strip()))
                ret += 1

        if ret == 0:
            print("\t\tPassed")
        else:
            print("\t\tFailed on {} lines".format(ret))
            ret_ += ret
    else:
        print("Run Failed".format(ret))

print("Test Finished with {} errors".format(ret_))

