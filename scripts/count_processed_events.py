


def run(args) :
    import tarfile
    import os, glob

    dir = args[1]

    event_count = 0
    print(os.listdir(dir))
    for subdir in os.listdir(dir) :
        if not subdir.isnumeric() :
            continue
        for path in glob.glob(dir+'/'+subdir+'/files_*larcv.csv.gz.tar') :
            local_count = 0
            with tarfile.open(path) as f :
                for ffinfo in f :
#                    print(ffinfo)
                    with tarfile.open(fileobj=f.extractfile(ffinfo)) as ff :
                       local_count += len(ff.getnames())
            print(path, local_count)
            event_count += local_count

    print(f'Total number of events: {event_count}')


import sys
if __name__ == '__main__':
    print('Running processing...')
    run(sys.argv)
