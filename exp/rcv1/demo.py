import os
import sys

#opt_list = ['SGD','STG','RDA','Ada-FOBOS','Ada-RDA', 'AROW', 'SSAROW', 'ASAROW']
opt_list = ['ASAROW']
#dataset_list = ['rcv1','real-sim','text','aut','pcmac','physic']
dataset_list = ['rcv1']

rootDir = '/home/matthew/work/Data/'

if len(sys.argv) == 2:
    dataset = sys.argv[1]
else:
    dataset = 'rcv1'

for dataset in dataset_list:
    test_file = ''
    if dataset == 'a6a':
        train_file = rootDir + 'uci/a6a'
        test_file = rootDir + '/uci/a6a.t'
    
    elif dataset == 'a9a':
        train_file = rootDir + 'uci/a9a'
        test_file = rootDir + 'uci/a9a.t'
    
    elif dataset == 'rcv1':
        train_file = rootDir + 'rcv1/rcv1.train' 
        test_file = rootDir + '/rcv1/rcv1.test'
    
    elif dataset == 'real-sim':
        train_file = rootDir + 'real-sim/real_sim_train'
        test_file  = rootDir + 'real-sim/real_sim_test'
    elif dataset == 'text':
        train_file = rootDir + 'text/text_train'
        test_file = rootDir + 'text/text_test' 
    
    elif dataset == 'aut':
        train_file = rootDir + 'aut/aut_train'
        test_file = rootDir + 'aut/aut_test' 
    
    elif dataset == 'pcmac':
        train_file = rootDir + 'pcmac/pcmac_train'
        test_file = rootDir + 'pcmac/pcmac_test' 
    
    elif dataset == 'news':
        train_file = rootDir + 'news/news_train'
        test_file = rootDir + 'news/news_test' 
    
    elif dataset == 'physic':
        train_file = rootDir + 'physic/physic_train'
        test_file  = rootDir + 'physic/physic_test' 
    
    elif dataset == 'kdda':
        train_file = rootDir + 'kdda/algebra/kdda'
        test_file = rootDir + 'kdda/algebra/kdda.t'
    
    elif dataset == 'epsilon':
        train_file = rootDir + 'epsilon/epsion_normalized'
        test_file = rootDir + 'epsilon/epsion_normalized.t'
    
    else:
        print 'unrecoginized dataset'
        sys.exit()
    
    
    cmd_data = ' -i %s' %train_file 
    cache_train_file = train_file + '_cache'
    cmd_data += ' -c %s' %cache_train_file
    
    if len(test_file) > 0:
        cache_test_file = test_file + '_cache'
        cmd_data += ' -t %s' %test_file + ' -tc %s' %cache_test_file
    else:
        cache_test_file = ''
    
    dst_folder = './result/' 
    os.system("mkdir -p %s" %dst_folder)
    
    #analyze dataset
    dataset_info_file = train_file + '_info.txt'
    if os.path.exists(dataset_info_file) == False:
        print 'analyze dataset'
        cmd = '../analysis %s' %train_file +' >> %s' %dataset_info_file
        print cmd
        os.system(cmd)
    
    #train model
    for opt in opt_list:
        print '-----------------------------------'
        print 'Experiment on %s' %opt
        print '-----------------------------------'
        if opt == 'vw':
            cmd = 'python run_vw.py %s' %dst_folder 
            cmd += ' %s' %train_file + ' %s'  %cache_train_file
            cmd += ' %s' %test_file + ' %s' %cache_test_file
            os.system(cmd)
    
        else:
            cmd = 'python run_experiment.py %s' %opt  + ' %s' %dst_folder
            cmd += cmd_data
            os.system(cmd)
    
    sys.exit()
    opt_list_file = './%s' %dst_folder + '/opt_list.txt' 
    #clear the file if it already exists
    open(opt_list_file,'w').close()
    
    try:
        file_handle = open(opt_list_file,'w')
        for opt in opt_list:
            file_handle.write(opt + '.txt\n')
    except IOError as e:
        print "I/O error ({0}): {1}".format(e.errno,e.strerror)
        sys.exit()
    else:
        file_handle.close()
