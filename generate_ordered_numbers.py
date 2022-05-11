import random 
import sys 

n = int(sys.argv[1])

with open("sample_inputs_ordered.txt", 'w') as f:
    f.write(str(n))
    f.write("\n")
    for i in range(2*n):
        s = str(range(1, n+1)).replace("[","")
        s = s.replace("]","")
        s = s.replace(",","")
        f.write(s)
        f.write("\n")
    f.close()
    