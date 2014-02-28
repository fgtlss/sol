#!/usr/bin/env python
"""bach cross validation"""
import os
import dataset
import exe_path

opt_list = ['FOFS','AROW','SGD']

ds_list = ['relathe','pcmac','basehock','ccat','aut','real-sim']
ds_list = ['news','rcv1','url']
ds_list = ['synthetic_100_10K_100K','synthetic_200_20K_100K']
fold_num =5

const_eta_search = '0.03125:2.0:32'
eta_search = '0.25:2.0:256'
delta_search = '0.03125:2.0:32'
r_search = '0.25:2.0:256'
delta_ofs_search = '0.0003125:2:0.32'

for dt in ds_list:
    split_list = dataset.split_dataset(dt,fold_num)
    #train model
    for opt in opt_list:
        print '----------------------------------------------'
        print 'Cross validation on %s' %dt + ' with %s' %opt
        print '----------------------------------------------'

        cmd = exe_path.cv_script + ' {0} {1} {2} '.format(dt,opt,fold_num)

        if 'AROW' in opt: 
            cmd += ' {0} {1}'.format('-r',r_search)
        elif 'Ada' in opt: 
            cmd += ' {0} {1}'.format('-delta',delta_search)
            cmd += ' {0} {1}'.format('-eta',const_eta_search)
        elif opt == 'OFSGD':
            cmd += ' {0} {1}'.format('-delta',delta_ofs_search)
            cmd += ' {0} {1}'.format('-eta',const_eta_search)
        else:
            cmd += ' {0} {1}'.format('-eta',eta_search)
        print cmd
        os.system(cmd)
    #del the cv files
    for split_item in split_list:
        os.system('rm -f %s' %split_item)
        cache_file = split_item + '_cache'
        os.system('rm -f %s' %cache_file)


