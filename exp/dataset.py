#!/usr/bin/env python
"""dataset list"""

import sys
import os
import platform

# windows
if platform.system() == 'Windows':
    rootDir = 'D:/Coding/SOL/data/'
elif platform.system() == 'Linux':
    rootDir = '/home/matthew/work/Data/'
else:
    print 'system type is not supported:'
    sys.exit()

#analyze dataset
def analyze(file_name):
    info_name = file_name + '_info.txt'
    if os.path.exists(info_name) == False:
        print 'analyze %s' %file_name
        cmd = '..' + os.sep + 'analysis %s' %file_name +' >> %s' %info_name
        print cmd
        os.system(cmd)

def get_file_name(dataset, task = 'train'):
    if dataset == 'rcv1':
        train_file = 'rcv1/rcv1.train' 
        test_file = 'rcv1/rcv1.test'
    elif dataset == 'news':
        train_file = 'news/news_train'
        test_file = 'news/news_test' 
    elif dataset == 'real-sim':
        train_file = 'real-sim/real-sim_train'
        test_file = 'real-sim/real-sim_test' 
    elif dataset == 'epsilon':
        train_file = 'epsilon/epsilon_normalized'
        test_file = 'epsilon/epsilon_normalized.t'
    elif dataset == 'url':
        train_file = 'url_combined/url_train'
        test_file  = 'url_combined/url_test'
    elif dataset == 'webspam':
        train_file = 'webspam/webspam_unigram_train'
        test_file  = 'webspam/webspam_unigram_test'
    elif dataset == 'webspam_trigram':
	if task == 'cv':
            train_file = 'webspam_trigram_15K/webspam_trigram_15K_train'
            test_file  = 'webspam_trigram_15K/webspam_trigram_15K_test'
	else:
	    train_file = 'webspam_trigram/webspam_trigram_train'
            test_file  = 'webspam_trigram/webspam_trigram_test'
    elif dataset =='MNIST':
        train_file = 'MNIST/train67'
        test_file  = 'MNIST/test67'
    elif dataset == 'aut':
        train_file = 'aut/aut_train'
        test_file = 'aut/aut_test'
    elif dataset == 'news20':
        train_file = 'news20/news20_train'
        test_file = 'news20/news20_test'
    elif dataset == 'gisette':
        train_file = 'gisette/gisette_scale'
        test_file = 'gisette/gisette_scale.t'
    elif dataset == 'pcmac':
        train_file = 'pcmac/pcmac_train'
        test_file = 'pcmac/pcmac_test'
    elif dataset == 'physic':
        train_file = 'physic/physic_train'
        test_file = 'physic/physic_test'
    elif dataset == 'real-sim':
        train_file = 'real-sim/real-sim_train'
        test_file = 'real-sim/real-sim_test'
    else:
        print 'unrecoginized dataset'
        sys.exit()

    train_file = rootDir + train_file
    test_file = rootDir + test_file
    if os.sep != '/':
        train_file = train_file.replace('/', os.sep)
        test_file = test_file.replace('/', os.sep)

    path_list = ['','']
    path_list[0] = train_file
    path_list[1] = test_file

    return path_list

def get_cv_data_list(dataset, fold_num):
    path_list = get_file_name(dataset,'cv')
    train_file = path_list[0]
    test_file = path_list[0]

    # count number of lines
    count_cmd = 'wc -l %s' %train_file
    count_handler = os.popen(count_cmd)
    line_num = int(count_handler.read().split()[0])
    count_handler.close()

    #split the train_data into fold_num pieces
    split_line_num = int(line_num / fold_num)

    #catch all the sub files, note that fold-num must less 26
    split_list = []
    for k in range(0,fold_num):
        file_name = train_file + '_cva' + chr(ord('a') + k)
        os.system('rm -f %s' %file_name)
        split_list.append(file_name)

    split_cmd = 'split -l {0} {1} {2}_cv'\
            .format(split_line_num,train_file, train_file) 

    os.system(split_cmd)
    
    return split_list

def get_cmd_data_by_file(train_file, test_file):
    cache_train_file = train_file + '_cache'
    cmd_data = ' -i %s' %train_file + ' -c %s' %cache_train_file

    cache_test_file = test_file + '_cache'
    cmd_data += ' -t %s' %test_file + ' -tc %s' %cache_test_file

    return cmd_data

def get_cmd_data(dataset):
    path_list = get_file_name(dataset)
    train_file = path_list[0]
    test_file = path_list[0]

    analyze(train_file)
    analyze(test_file)

    return get_cmd_data_by_file(train_file, test_file)

def get_model_param(ds, opt):
    news = {'SGD':{'-eta':64},'STG':{'-eta':64},
            'Ada-FOBOS':{'-eta':2, '-delta':0.0625},
            'Ada-RDA':{'-eta':2, '-delta':0.0625},
            'SSAROW':{'-r':0.125}, 'ASAROW':{'-r':0.125},
            'CW-RDA':{'-r':0.0625}, 'RDA':{'-eta':256}}
    MNIST = {'SGD':{'-eta':1},'STG':{'-eta':1},
            'Ada-FOBOS':{'-eta':1, '-delta':8},
            'Ada-RDA':{'-eta':2, '-delta':1},
            'SSAROW':{'-r':0.0625}, 'ASAROW':{'-r':0.0625},
            'CW-RDA':{'-r':0.0625}, 'RDA':{'-eta':1}}
    rcv1 = {'SGD':{'-eta':128},'STG':{'-eta':128},
            'Ada-FOBOS':{'-eta':1, '-delta':0.125},
            'Ada-RDA':{'-eta':1, '-delta':0.0625},
            'SSAROW':{'-r':2}, 'ASAROW':{'-r':2},
            'CW-RDA':{'-r':2}, 'RDA':{'-eta':512}}
    url = {'SGD':{'-eta':16},'STG':{'-eta':16},
            'Ada-FOBOS':{'-eta':1, '-delta':0.0625},
            'Ada-RDA':{'-eta':1, '-delta':0.0625},
            'SSAROW':{'-r':0.0625}, 'ASAROW':{'-r':0.0625},
            'CW-RDA':{'-r':0.0625}, 'RDA':{'-eta':64}}
    aut = {'SGD':{'-eta':64},'STG':{'-eta':64},
                'Ada-FOBOS':{'-eta':1, '-delta':0.0625},
                'Ada-RDA':{'-eta':1, '-delta':0.125},
                'SSAROW':{'-r':0.5}, 'ASAROW':{'-r':0.5},
                'CW-RDA':{'-r':0.5}, 'RDA':{'-eta':512}}
    news20 = {'SGD':{'-eta':128},'STG':{'-eta':128},
                'Ada-FOBOS':{'-eta':1, '-delta':0.0625},
                'Ada-RDA':{'-eta':2, '-delta':0.0625},
                'SSAROW':{'-r':0.125}, 'ASAROW':{'-r':0.125},
                'CW-RDA':{'-r':0.0625}, 'RDA':{'-eta':256}}
    pcmac = {'SGD':{'-eta':4},'STG':{'-eta':4},
                'Ada-FOBOS':{'-eta':2, '-delta':1},
                'Ada-RDA':{'-eta':1, '-delta':0.25},
                'SSAROW':{'-r':2}, 'ASAROW':{'-r':2},
                'CW-RDA':{'-r':2}, 'RDA':{'-eta':8}}
    physic = {'SGD':{'-eta':256},'STG':{'-eta':256},
                'Ada-FOBOS':{'-eta':1, '-delta':0.125},
                'Ada-RDA':{'-eta':1, '-delta':0.0625},
                'SSAROW':{'-r':0.125}, 'ASAROW':{'-r':0.125},
                'CW-RDA':{'-r':0.5}, 'RDA':{'-eta':256}}
    realsim = {'SGD':{'-eta':512},'STG':{'-eta':512},
                'Ada-FOBOS':{'-eta':512, '-delta':0.0625},
                'Ada-RDA':{'-eta':512, '-delta':0.0625},
                'SSAROW':{'-r':4}, 'ASAROW':{'-r':4},
                'CW-RDA':{'-r':4}, 'RDA':{'-eta':256}}
    gisette = {'SGD':{'-eta':1},'STG':{'-eta':1},
                'Ada-FOBOS':{'-eta':2, '-delta':1.0},
                'Ada-RDA':{'-eta':32, '-delta':4},
                'SSAROW':{'-r':0.0625}, 'ASAROW':{'-r':0.0625},
                'CW-RDA':{'-r':0.0625}, 'RDA':{'-eta':256}}
    webspam_trigram = {'SGD':{'-eta':128},'STG':{'-eta':128},
            'Ada-FOBOS':{'-eta':4, '-delta':0.0625},
            'Ada-RDA':{'-eta':8, '-delta':0.0625},
            'SSAROW':{'-r':0.0625}, 'ASAROW':{'-r':0.0625},
            'CW-RDA':{'-r':0.0625},'RDA':{'-eta':512}}
    ds_opt_param = {'news':news,'MNIST':MNIST,'rcv1':rcv1,
            'url':url,'webspam_trigram':webspam_trigram,
            'aut':aut, 'news20':news20,'physic':physic,
            'pcmac':pcmac,'gisette':gisette,'real-sim':realsim}

    cmd = ''
    if ds in ds_opt_param.keys():
        if opt in ds_opt_param[ds].keys():
            for key,val in ds_opt_param[ds][opt].iteritems():
                cmd += ' {0} {1} '.format(key, val)
        else:
            print opt + ' not unrecognized!'
    else:
        print ds + ' not unrecognized!'
    return cmd
