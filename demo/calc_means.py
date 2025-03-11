# This Python file uses the following encoding: utf-8
import os.path
import sys
import json
import glob
import numpy as np

def getMeta(npy_filename):
    json_filename=os.path.splitext(npy_filename)[0]+".json"
    with open(json_filename, "r", encoding='utf8') as f:
        meta=json.load(f)
    return meta

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
    if(len(sys.argv)<4):
        print("""
usage: calc_mean <1st point> <last point> <file1>.npy [additional files ...]
example: calc_mean 0 100 PM_1_1_1_Imon.npy
        """,file=sys.stderr)
        sys.exit(-1)
    p1=int(sys.argv[1])
    p2=int(sys.argv[2])
    filelist=[]
    for index in range(3,len(sys.argv)):
        filelist+=glob.glob(sys.argv[index])
    if(len(filelist)==0):
        print("no matching files found",file=sys.stderr)
        sys.exit(-1)
    filelist.sort(key=TraceKey)
    for npy_filename in filelist:
        basename=os.path.basename(os.path.splitext(npy_filename)[0])
        meta=getMeta(npy_filename)
        y_w=np.load(npy_filename)
        print(basename, meta['params']['sweep']['Rel. Sweep Time'], np.mean(y_w[p1:p2]), sep='\t')
