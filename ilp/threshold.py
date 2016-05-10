#!/usr/bin/python


import sys
import time
import seeds

if __name__ == "__main__":
    
    if len(sys.argv) < 4:
        sys.exit(-1)
    
    F = seeds.SeedsFamily(sys.argv[3:])

    sys.stdout = open("/dev/null", "w")
    start = time.time()
    t = F.threshold(int(sys.argv[1]), int(sys.argv[2]));
    stop = time.time()
    sys.stdout = open("/dev/stdout", "w")

    print str(t) + "\t" + str(stop - start)

