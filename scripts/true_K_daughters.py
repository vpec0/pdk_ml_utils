import uproot as ur
import awkward as ak
import numpy as np
from matplotlib import pyplot as plt


outpref = 'plots/mc_truth/'
mc_truth_base = 'simb::MCParticles_largeant__G4./simb::MCParticles_largeant__G4.obj/simb::MCParticles_largeant__G4.obj.f'


def process_tree(t):
    ## PROBLEM: cannot read in trajectoyr => can't get energy
    to_read={key:mc_truth_base+key for key in ['pdgCode', 'process', 'endprocess', 'daughters','mother']} #, 'trajectory']}
    #to_read['trajectory'] += '.ftrajectory'

    # for key,val in to_read.items() :
    #     print(key, t[val].interpretation)

    b = t.arrays(to_read.keys(), aliases=to_read) #, entry_stop=10)


    #traj       = b['trajectory']
    pdg        = b['pdgCode']
    endprocess = b['endprocess']
    process    = b['process']
    mother     = b['mother']-1
    daughters  = b['daughters']-1

    d_pdg = ak.unflatten(pdg[ak.flatten(daughters, axis=-1)], ak.ravel(ak.num(daughters, axis=-1)),axis=-1)
    # print('Mother:', mother[0,0:10])
    # #print('TrkId', trkId[0,0:10])
    # print('Pdg', pdg[0,0:10])
    # print('Daughters:', daughters[0,0:4])
    # print('D pdg:', d_pdg[0,0:4])

    mcIdx = ak.local_index(pdg)
    kaon_mask = (mother == -1) & (pdg == 321) # only interested in primary kaons
    dk_kaon_mask = kaon_mask & (endprocess == 'Decay')
    dk_k_daughters = daughters[dk_kaon_mask]
    dk_k_daughters_mother = ak.unflatten(mother[ak.flatten(dk_k_daughters, axis=-1)], ak.ravel(ak.num(dk_k_daughters, axis=-1)),axis=-1)
    dk_k_daughters_pdg = ak.unflatten(pdg[ak.flatten(dk_k_daughters, axis=-1)], ak.ravel(ak.num(dk_k_daughters, axis=-1)),axis=-1)
    dk_k_daughters_process = ak.unflatten(process[ak.flatten(dk_k_daughters, axis=-1)], ak.ravel(ak.num(dk_k_daughters, axis=-1)),axis=-1)
    dk_k_dk_daughters = dk_k_daughters[dk_k_daughters_process=='Decay']
    dk_k_dk_daughters_pdg = ak.unflatten(pdg[ak.flatten(dk_k_dk_daughters, axis=-1)], ak.ravel(ak.num(dk_k_dk_daughters, axis=-1)),axis=-1)
    print(f'mcIdx[dk_kaon_mask]   = {mcIdx[dk_kaon_mask]  }' )
    print(f'dk_k_daughters        = {dk_k_daughters       }' )
    print(f'dk_k_daughters_mother = {dk_k_daughters_mother}' )
    print(f'dk_k_daughters_pdg    = {dk_k_daughters_pdg   }' )
    print(f'dk_k_dk_daughters_pdg = {dk_k_dk_daughters_pdg}' )


    nkaons = ak.sum(kaon_mask, axis=-1)
    print(f'{nkaons =}')
    print(f'{nkaons[nkaons>1] =}')



    #is_true_daughter =
    # electron_mask = pdg[k_daughters] == 11
    # k_daughter_electrons = k_daughters[electron_mask]
    # no_decay_mask = process[k_daughter_electrons] != 'Decay'

    #plt.hist(



import sys, os

def run(args):
    print('Input:', args[1])
    #print(mlp.matplotlib_fname())

    # make sure the output directory exists
    print('Output plots dir:', os.path.dirname(outpref))
    os.makedirs(os.path.dirname(outpref), exist_ok=True)

    with ur.open(args[1]+':Events') as tree:
        #test(tree)
        process_tree(tree)


if __name__ == '__main__':
    print('Running processing...')
    run(sys.argv)
