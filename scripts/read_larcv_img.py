import uproot as ur
import awkward as ak
import sys




def run(args) :
    fname = args[0]

    tree = ur.open(fname+':image2d_tpc_tree')
    b = tree['image2d_tpc_branch/_image_v/_image_v._img']

    # try to read in images from the 1st event
    images = b.array(entry_stop=1)
    print(f'{tree.num_entries = }')
    print(ak.num(images, axis=-1))




if __name__ == '__main__':
    print('Running processing...')
    run(sys.argv[1:])
