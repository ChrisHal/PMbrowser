import os.path
import sys
import json
import numpy as np

def getMeta(npy_filename):
    json_filename=os.path.splitext(npy_filename)[0]+".json"
    with open(json_filename, "r", encoding='utf8') as f:
        meta=json.load(f)
    return meta


if __name__ == '__main__':
    if(len(sys.argv)<4):
        print("""
usage: calc_mean <1st point> <last point> <file>.npy
example: calc_mean 0 100 PM_1_1_1_Imon.npy
        """,file=sys.stderr)
        sys.exit(-1)
    p1=int(sys.argv[1])
    p2=int(sys.argv[2])
    npy_filename=sys.argv[3]
    meta=getMeta(npy_filename)
    data=np.load(npy_filename)
    d_mean=np.mean(data[p1:p2])
    data_unit=meta['unit_y']
    print("mean =", d_mean, data_unit)
    
     
