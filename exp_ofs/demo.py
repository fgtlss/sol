#!/usr/bin/env python
import os
import sys
import dataset
import run_util
import sol_shuffle
import run_experiment
import run_vw
import run_liblinear
import run_fgm
import run_mRMR


#algorithm list
#batch algorithm
opt_list = ['liblinear','fgm','mRMR']
#online algorithm
opt_list = ['SOFS','FOFS','PET']

#dataset list
#synthetic data
ds_list = ['synthetic_100_10K_100K','synthetic_200_20K_100K']
#medium data
ds_list = ['relathe','pcmac','basehock','ccat','aut','real-sim']
#large data
ds_list = ['news','rcv1']

#number of times to randomize a dataset for averaged results
rand_num = 10
#extra command sent to SOL
extra_cmd = ' -loss Hinge -norm '

#whether need to cache the dataset for fast processing speed
is_cache = False
#whether to use the default parameter settings of each algorithm, otherwise,
#parameters will obtained from get_model_param in dataset.py
is_default_param = False

def add_to_dict(opt, result_all, result_once):
    if opt in result_all.keys(): #add to previous result
        rows = len(result_all[opt])
        cols = len(result_all[opt][0])
        for k in range(0,rows):
            for m in range(0,cols):
                result_all[opt][k][m] += float(result_once[k][m])
    else:
        result_all[opt] = result_once
        rows = len(result_once)
        cols = len(result_once[0])
        for k in range(0,rows):
            for m in range(0,cols):
                result_all[opt][k][m] = float(result_once[k][m])

        result_all[opt] = result_once

    return result_all

#train model
def train_model(path_list,dst_folder):
    train_file = path_list[0]
    test_file = path_list[1]

    result_all = {}
    #random the file
    if rand_num > 1:
        rand_file = train_file + '_rand'  
    else:
	    rand_file = train_file

    rand_file_cache = rand_file + '_cache'

    for k in range(0,rand_num):
        if rand_num > 1:
            print 'shuffle datset...'
            sol_shuffle.sol_shuffle(train_file, rand_file)

        cmd_data = dataset.get_cmd_data_by_file(rand_file, test_file, is_cache)
        dataset.analyze(rand_file);

        for opt in opt_list:
            print '-----------------------------------'
            print ' Experiment on %s' %opt + ' Random %d' %k 
            print '-----------------------------------'

            if opt == 'vw':
                result_file = 'vw_result_%d' %k + '.txt'
                result_once = run_vw.run_vw(rand_file, test_file, ds, result_file, is_cache)
            elif  opt == 'liblinear':
                result_file = 'liblinear_result_%d' %k + '.txt'
                result_once = run_liblinear.run_liblinear(rand_file, test_file, ds, result_file)
            elif opt == 'fgm':
                result_file = 'fgm_result_%d' %k + '.txt'
                result_once = run_fgm.run_fgm(rand_file, test_file, ds, result_file)
            elif opt == 'mRMR':
                result_file = dst_folder + '/%s' %opt + '_result_%d' %k + '.txt'
                result_once = run_mRMR.run_mRMR(rand_file, test_file, ds, result_file)

                print '\nparsing result...'
                #write the result to file
                parse_file = dst_folder +'/%s' %opt + '_%d' %k + '.txt'

                result_once2 = run_util.parse_result(result_file, parse_file);
                bs_num = len(result_once)
                if bs_num != len(result_once2):
                    print 'inconsistent parsing result'
                for m in range(0,bs_num):
                    result_once[m][0] = result_once2[m][0]
                    result_once[m][1] = result_once2[m][1]
                    if result_once[m][2] == 0:
                        result_once[m][2] = result_once2[m][2]
                    if result_once[m][3] == 0:
                        result_once[m][3] = result_once2[m][3]
            else:
                result_file = dst_folder + '/%s' %opt + '_result_%d' %k + '.txt'

                cmd = cmd_data
                cmd += extra_cmd
                if is_default_param == False:
                    cmd += dataset.get_model_param(ds, opt)

                run_experiment.run_experiment(opt,result_file,ds, cmd)

                print '\nparsing result...'
                #write the result to file
                parse_file = dst_folder +'/%s' %opt + '_%d' %k + '.txt'

                result_once = run_util.parse_result(result_file, parse_file);
            result_all = add_to_dict(opt,result_all, result_once)

        #remove previous file
        if rand_num > 1:
            os.system('rm -f %s' %rand_file_cache)
            os.system('rm -f %s' %rand_file)


    #average the result
    for opt in opt_list:
        rows = len(result_all[opt])
        cols = len(result_all[opt][0])

        for k in range(0,rows):
            for m in range(0,cols):
                result_all[opt][k][m] /= rand_num

    return result_all 

for ds in ds_list:
    path_list = dataset.get_file_name(ds)
    dst_folder = ds
    dst_folder = ds
    #os.system("mkdir %s" %dst_folder)
    run_util.create_dir(dst_folder)

    result_all = train_model(path_list, dst_folder)

    for key,val in result_all.iteritems():
        #write the result to file
        parse_file = dst_folder +'/%s' %key + '.txt'
        run_util.write_parse_result(val,parse_file)

    opt_list_file = '%s' %dst_folder + os.sep + 'opt_list.txt' 

    try:
        file_handle = open(opt_list_file,'w')
        for opt in opt_list:
            file_handle.write(opt + '.txt\n')
    except IOError as e:
        print "I/O error ({0}): {1}".format(e.errno,e.strerror)
        sys.exit()
    else:
        file_handle.close()

