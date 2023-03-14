import os.path
import sys
import json
import glob
import numpy as np
import matplotlib.pyplot as plt

def getMeta(npy_filename):
    json_filename=os.path.splitext(npy_filename)[0]+".json"
    with open(json_filename, "r", encoding='latin1') as f:
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
        print("""
usage: plot_traces <pattern1>.npy [<pattern2>.npy ...]
example: plot_traces "PM_1_*V*.npy" "PM_2_*Vm.npy"
        """,file=sys.stderr)
        sys.exit(-1)
    fig, ax = plt.subplots()
    filelist=[]
    for index in range(1,len(sys.argv)): 
        filelist+=glob.glob(sys.argv[index])
    for npy_filename in filelist: 
        basename=os.path.basename(os.path.splitext(npy_filename)[0])
        meta=getMeta(npy_filename)
        x_w=createX(meta)
        y_w=np.load(npy_filename)
        ax.plot(x_w,y_w,label=basename)
    ax.legend()
    ax.set_xlabel('['+meta['unit_x']+']')
    ax.set_ylabel('['+meta['unit_y']+']')
    plt.show()

