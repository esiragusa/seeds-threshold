#!/usr/bin/python

import sys
import seeds


if __name__ == "__main__":
    
    if len(sys.argv) < 3:
        sys.exit(-1)
    
    F = seeds.SeedsFamily(sys.argv[2:])
    
    print 'Minimum Non-Detected Error = ' + str(F.minerror(int(sys.argv[1])))
