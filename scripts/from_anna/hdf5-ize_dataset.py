import h5py
import numpy as np
import uproot as ur
import awkward as ak
#import torch
import matplotlib.pyplot as plt


def get_meta(branch):
    image_meta = {'image_id': '',
                  'origin.x': '',
                  'origin.y': '',
                  'width': '',
                  'height': '',
                  'col_count': '',
                  'row_count': '',
                  'plane': ''}

    for key in branch.keys():
        if '_meta.' in key:
            if 'x' not in key and 'y' not in key:
                key_parts = key.split('.')
                image_meta[key_parts[-1].lstrip('_')] = key
            else:
                key_parts = key.split('_')
                image_meta[key_parts[-1]] = key

    return image_meta


def get_comp_from_root(rootf, prec, next_):
    tree = rootf['image2d_tpc_tree']
    image_sup_branch = tree['_image_v']
    image_branch = tree['_image_v/_image_v._img']

    branch_entries = image_branch.array(entry_start=prec, entry_stop=next_)
    linear_data = branch_entries[0]

    image_meta = get_meta(image_sup_branch)
    col_count = image_sup_branch[image_meta['col_count']].array(entry_start=prec, entry_stop=next_)[0]
    row_count = image_sup_branch[image_meta['row_count']].array(entry_start=prec, entry_stop=next_)[0]
    result = []
    for i in range(3) :
        mdata = ak.to_numpy(linear_data[i]).reshape(col_count[i], row_count[i])
        nz_indices = np.transpose(np.nonzero(mdata))
        nz_x = [index[0] for index in nz_indices]
        nz_y = [index[1] for index in nz_indices]
        nz_values = mdata[nz_indices[:, 0], nz_indices[:, 1]]
        result.append((nz_x, nz_y, nz_values))
    return result


# def coo_to_torch(hdf5_path, image_key):
#     with h5py.File(hdf5_path, 'r') as hdf5_file:
#         if image_key in hdf5_file:
#             dset = hdf5_file[image_key]
#             nz_x = dset['nz_x'][:]
#             nz_y = dset['nz_y'][:]
#             values = torch.FloatTensor(dset['value'][:])
#             indices = torch.LongTensor([nz_x, nz_y])
#             rows, cols = max(nz_x) + 1, max(nz_y) + 1
#             sp_tensor = torch.sparse_coo_tensor(indices, values, torch.Size([rows, cols]))

#             return sp_tensor


def create_hdf5_from_root(rootf_path, hdf5_path):
    dtype = np.dtype([
        ('nz_x', np.uint32),
        ('nz_y', np.uint32),
        ('value', np.float32)])

    with ur.open(rootf_path) as rootf:
        with h5py.File(hdf5_path, 'a') as hdf5_file:

            for i in range(10):
                by_plane = get_comp_from_root(rootf, i, i + 1)
                iplane = -1
                for nz_x, nz_y, nz_values in by_plane :
                    iplane += 1
                    components = np.array(list(zip(nz_x, nz_y, nz_values)), dtype=dtype)
                    dset_name = f"image_plane{iplane}_{i + 1}"
                    if dset_name not in hdf5_file:
                        hdf5_file.create_dataset(dset_name, data=components, compression='gzip',
                                                 compression_opts=4, chunks=True)


def print_hdf5_dirtree(hdf5_path):
    with h5py.File(hdf5_path, 'r') as file:
        def recurse_through_group(group, indent=0):
            for key, item in group.items():
                print('│   ' * indent + '├── ' + key)
                if isinstance(item, h5py.Group):
                    recurse_through_group(item, indent + 1)
                elif isinstance(item, h5py.Dataset):
                    print('│   ' * (indent + 1) + '└── {} (shape: {}, dtype: {})'.format(key, item.shape, item.dtype))

                    print('│   ' * (indent + 2) + '├── nz_x')
                    print('│   ' * (indent + 2) + '├── nz_y')
                    print('│   ' * (indent + 2) + '└── nz_values')

        recurse_through_group(file)


def plot_im_from_hdf5(hdf5_path, image_key):
    with h5py.File(hdf5_path, 'r') as hdf5_file:
        if image_key in hdf5_file:
            dset = hdf5_file[image_key]
            nz_x = dset['nz_x'][:]
            nz_y = dset['nz_y'][:]
            values = dset['value'][:]
            rows, cols = max(nz_x) + 1, max(nz_y) + 1
            dense_matrix = np.zeros((rows, cols))
            dense_matrix[nz_x, nz_y] = values
            plt.imshow(dense_matrix, cmap='viridis', interpolation='nearest', aspect='auto')
            plt.colorbar()
            plt.tight_layout()
            plt.show()


if __name__ == '__main__':
    import sys, os

    rootf_path = sys.argv[1]
    hdf5_path = sys.argv[2]

    print(f'{rootf_path = }')
    print(f'{hdf5_path = }')
    create_hdf5_from_root(rootf_path, hdf5_path)


    # print_hdf5_dirtree(hdf5_path)
    # plot_im_from_hdf5(hdf5_path, 'image_2')
    # sp_tensor = coo_to_torch(hdf5_path, 'image_2')
