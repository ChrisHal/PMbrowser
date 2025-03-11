import os.path
import sys
import json
import glob
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

def getNpyBasename(npy_filename):
    return os.path.basename(os.path.splitext(npy_filename)[0])

class TraceKey:
    """Key useable for sorting tracenames."""
    def __init__(self,npy_filename):
        c=getNpyBasename(npy_filename).split('_')
        self.prefix=c[0]
        self.group=int(c[1])
        self.series=int(c[2])
        self.sweep=int(c[3])
        self.trace=c[4]
        
    def __lt__(self, other):
        if(self.prefix<other.prefix):
            return True
        if(self.prefix>other.prefix):
            return False
        if(self.group<other.group):
            return True
        if(self.group>other.group):
            return False
        if(self.series<other.series):
            return True
        if(self.series>other.series):
            return False
        if(self.sweep<other.sweep):
            return True
        if(self.sweep>other.sweep):
            return False
        if(self.trace<other.trace):
            return True
        return False

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
    if(len(filelist)==0):
        print("no matching files found",file=sys.stderr)
        sys.exit(-1)
    filelist.sort(key=TraceKey)
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

