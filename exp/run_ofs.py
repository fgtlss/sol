#This script is to run experiment automatically to test the performance of the algorithm

import sys
import os

import util

#run the online feature selection experiment
#@param dataset: DataSet instance
#@param model: model to train 
#@param config: cnfiguration to train the model
#@param output_file: output file to save the results
def run(dataset, model, config, output_file):
    #ofs executable
    if util.get_platform() == 'Windows':
        ofs_exe = r'..\install\bin\SOL.exe'
    else:
        ofs_exe = '../install/bin/SOL'

    #get the dimension of the data
    data_dim = dataset.dim

    sel_feat_num_list = dataset.l0_list

    #evaluate the result
    cmd_postfix = ' >> %s' %output_file

    dt_cmd = dataset.get_train_cmd(config['cache'], config['rand_num'])
    if dataset.class_num > 2:
        if model == 'SOFS':
            loss_cmd = ' -loss MaxScoreSquaredHinge '
        else:
            loss_cmd = ' -loss {0} '.format(config['mc_loss'])
    else:
        if model == 'SOFS':
            loss_cmd = ' -loss SquaredHinge '
        else:
            loss_cmd = ' -loss {0} '.format(config['bc_loss'])

    norm_cmd = ' -norm ' if config['norm'] == True else '' 

    cmd_prefix  = ofs_exe + dt_cmd + loss_cmd + norm_cmd  + ' -m %s ' %model

    for sel_num in sel_feat_num_list:
        cmd = cmd_prefix + ' -k %d' %sel_num + cmd_postfix
        print cmd
        os.system(cmd)

    #parse the result
    result = util.ResultItem()
    result.parse_ofs_result(output_file)

    return result
