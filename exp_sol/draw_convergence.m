function draw_convergence(dataset, type, sparsity, ymin, ymax)
close all;
if (~exist('ymin','var'))
    ymin = 0;
end
if (~exist('ymax','var'))
    ymax = 0;
end
if (~exist('sparsity','var'))
    sparsity = 0;
end

color_list = {'r','m','b','black'};
color_num = size(color_list,2);
shape_list = {'+','s','o','*','v','d'};
shape_num = size(shape_list,2);

cur_color_index = 1;
cur_shape_index = 1;

folder_name = strcat(dataset,'/');
mkdir figs
opt_list_file = strcat(folder_name,'opt_list.txt');
if type == 'TG'
    opt_list = {'SSAROW.txt';'FOBOS.txt';'STG.txt';'Ada-FOBOS.txt'};
elseif type =='DA'
    opt_list = {'CW-RDA.txt';'RDA.txt';'Ada-RDA.txt'};
elseif type == 'FS'
    opt_list = {'AROW-FS.txt';'SGD-FS.txt';'OFSGD.txt'};
elseif type == 'CMP'
    opt_list = {'AROW-FS.txt';'AROW-TG.txt';'AROW-DA.txt'};
end
%opt_list = textread(opt_list_file,'%s');

opt_num = size(opt_list,1);

legend_content = cell(1,opt_num);


for k = 1:1:opt_num
    result_file = opt_list{k,1};
    opt_name = my_split(result_file,'.');
    opt_name = opt_name{1,1};
    if strcmp(opt_name, 'SSAROW') 
        legend_content{1,k} = 'AROW-TG';
    elseif strcmp(opt_name,'CW-RDA')
        legend_content{1,k} = 'AROW-DA';
    elseif strcmp(opt_name,'ASAROW')
        legend_content{1,k} = 'AROW-FS';
    else
        legend_content{1,k} = opt_name;
    end

    result = load(strcat(folder_name, result_file));
   
    sparse_vec = result(:,3); 
    conv_vec = result(:,5:end);
    index = 0;
    if sparsity > 0
        abs_diff = abs(sparse_vec - sparsity);
        [C I] = min(abs_diff);
        conv_vec = conv_vec(I(1),:);
    else
        conv_vec = conv_vec(1,:);
    end

	color_shape = strcat(color_list{1,cur_color_index}, '-');
    color_shape = strcat(color_shape, shape_list{1,cur_shape_index});
   
	figure(1)
	hold on
    grid on
    box on
    sample_times = size(conv_vec);    
    sample_num_vec = zeros(sample_times);
    sample_times = sample_times(2);
    for m = 1:1:sample_times
        sample_num_vec(m) = 2^m;
    end
	plot(sample_num_vec, conv_vec, color_shape,'LineWidth',2,'markersize',5);
    
    
    cur_color_index = cur_color_index + 1;
    if cur_color_index > color_num
        cur_color_index = 1;
    end

    cur_shape_index = cur_shape_index + 1;
    if cur_shape_index > shape_num
        cur_shape_index = 1;
    end

end
folder_name = strcat('figs/',dataset);
folder_name = strcat(folder_name,'-');
figure(1) %convergence
%title('convergence analysis', 'fontsize',14)
if ymax ~= 0
    ylim([ymin ymax])
end
ylabel('learning error rate', 'fontsize',14)
xlabel('Number of Samples', 'fontsize',14)
legend(legend_content,'location','northeast')