import tarfile
import gzip as gz
import os, glob
from scipy import sparse as sp
import numpy as np
import pandas as pd
from matplotlib import pyplot as plt
from matplotlib import cm

def goBack(f, nlines) :
    counter = nlines
    while nlines != 0 :
        if f.read(1) is r'\n' :
            counter -= 1
        f.seek(f.tell()-2)



def getEvent(topdirname):
    output = []

    # go over all sub-directories
    for subdir in os.listdir(topdirname) :
        if not subdir.isnumeric() :
            continue
        # go over all tar files
        for path in glob.glob(topdirname+'/'+subdir+'/files_*larcv.csv.gz.tar') :
            with tarfile.open(path) as f :
                print(path)
                # go over 2nd level of tar files: groups of events
                for ffinfo in f :
                    with tarfile.open(fileobj=f.extractfile(ffinfo)) as ff :
                        # go over 3rd level of files: individual events
                        for fffinfo in ff:
                            with tarfile.open(fileobj=ff.extractfile(fffinfo)) as fff :
                                iplane = -1
                                for planegzinfo in fff:
                                    iplane += 1
                                    planegz = fff.extractfile(planegzinfo)
                                    with gz.open(planegz, mode='rt') as planegzf :
                                        #print(planegzf.tell())
                                        firstline = planegzf.readline()
                                        #print(firstline,end='')
                                        nrows,ncols = tuple([int(i) for i in firstline.split() if i.isnumeric() ])
                                        print(f'Reading in {nrows} rows by {ncols} columns from plane {iplane}')
                                        df = pd.read_csv(planegzf, nrows=nrows*ncols, header=None)
                                        nparr = df.to_numpy().reshape((nrows,ncols))

                                        planegzf.seek(planegzf.tell() - 50)
                                        lines=planegzf.readlines()
                                        ROI = [[int(i) for i in lines[-j].split() if i.isnumeric() ] for j in [2,1] ]
                                        #print(ROI)

                                        output.append(sp.csr_matrix(nparr[ROI[1][0]:ROI[1][1],ROI[0][0]:ROI[0][1]]))

                    break
            break
        break
    return output


def plotImages(imgs):
    plt.ioff = True

    i = -1
    for planeimg in imgs:
        i += 1
        sp.save_npz(f'temp/temp{i}', planeimg)
        # convert to np array and plot with matplotlib
        plt.clf()
        max = abs(planeimg).max()
        norm = cm.colors.Normalize(vmax=max, vmin=-max)
        plt.imshow(planeimg.toarray(), norm=norm, cmap='RdYlBu')
        plt.colorbar()
        plt.savefig(f'evt_pl{i}.png')


def run(args) :

    dir = args[1]
    #print(os.listdir(dir))

    imgs = getEvent(dir)
    i = -1
    for planeimg in imgs:
        i += 1
        sp.save_npz(f'temp{i}', planeimg)

    #plotImages(imgs)

import sys
if __name__ == '__main__':
    print('Running processing...')
    run(sys.argv)
