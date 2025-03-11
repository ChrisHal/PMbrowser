import os.path
import sys
import json
import numpy as np
import matplotlib.pyplot as plt

def getMeta(npy_filename):
    json_filename=os.path.splitext(npy_filename)[0]+".json"
    with open(json_filename, "r", encoding='utf8') as f:
        meta=json.load(f)
    return meta

def createX(meta):
    x0=meta['x_0']
    deltax=meta['delta_x']
    numpnts=meta['numpnts']
    lastx=x0+deltax*numpnts
    x=np.linspace(x0,lastx,numpnts)
    return x

if __name__ == '__main__':
    if(len(sys.argv)<2):
        print("usage: multiplot_traces <filename1>.npy [<file2>.npy ...]",file=sys.stderr)
        sys.exit(-1)
    fig, ax = plt.subplots()
    for index in range(1,len(sys.argv)): 
        npy_filename=sys.argv[index]
        basename=os.path.basename(os.path.splitext(npy_filename)[0])
        meta=getMeta(npy_filename)
        x_w=createX(meta)
        y_w=np.load(npy_filename)
        ax.plot(x_w,y_w,label=basename)
    ax.legend()
    ax.set_xlabel('['+meta['unit_x']+']')
    ax.set_ylabel('['+meta['unit_y']+']')
    plt.show()

