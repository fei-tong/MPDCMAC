close all 
clc

file_num = [1 2 3 4 5];
pgi = [10 15 20 25 30];

dcpf_10_1c_nst_delivery_ratio = zeros(5,1);
mpdc_10_2c_nst_delivery_ratio = zeros(5,1);
mpdc_10_2c_st_delivery_ratio = zeros(5,1);
mpdc_10_3c_st_delivery_ratio = zeros(5,1);


output_data_folder = 'R_data';
data_format = '%s %s %d %s %f %d %d %d %d';

file_name = [output_data_folder,'/dcpf_10_1C_nst_10','s.txt'];
[~,~,~,~,scr_id,gen_num,grade,latency,~]=textread(file_name,data_format);
jf_array = [scr_id,gen_num];
    
for ii = file_num
    %% dcpf,sf = 10 
    file_name = [output_data_folder,'/dcpf_10_1C_nst_',int2str(pgi(ii)),'s.txt'];
    [~,~,~,~,scr_id,gen_num,grade,latency,~]=textread(file_name,data_format);
    tmp_array = [scr_id,gen_num];
    % get the min node id and the max node id
    min_id = min(scr_id);
    max_id = max(scr_id);
    total_gen_num = 0;
    total_receive_num = length(gen_num);
    for k = min_id:max_id
        d2= tmp_array(find(tmp_array(:,1) == k),:);
        total_gen_num = total_gen_num + d2(end,end);
    end
    disp([total_receive_num,total_gen_num]);
    dcpf_10_1c_nst_delivery_ratio(ii) = total_receive_num/total_gen_num;
    
    %% mpdc sf =10, with 2 channels,without stagger
    file_name = [output_data_folder,'/mpdc_10_2C_nst_',int2str(pgi(ii)),'s.txt'];
    [~,~,~,~,scr_id,gen_num,grade,latency,~]=textread(file_name,data_format);
    tmp_array = [scr_id,gen_num];
    % get the min node id and the max node id
    min_id = min(scr_id);
    max_id = max(scr_id);
    total_gen_num = 0;
    total_receive_num = length(gen_num);
    for k = min_id:max_id
        d2= tmp_array(find(tmp_array(:,1) == k),:);
        total_gen_num = total_gen_num + d2(end,end);
    end
    mpdc_10_2c_nst_delivery_ratio(ii) = total_receive_num/total_gen_num;
    
   %% mpdc sf =10, with 2 channels,with stagger
    file_name = [output_data_folder,'/mpdc_10_2C_st_',int2str(pgi(ii)),'s.txt'];
    [~,~,~,~,scr_id,gen_num,grade,latency,~]=textread(file_name,data_format);
    tmp_array = [scr_id,gen_num];
    % get the min node id and the max node id
    min_id = min(scr_id);
    max_id = max(scr_id);
    total_gen_num = 0;
    total_receive_num = length(gen_num);
    for k = min_id:max_id
        d2= tmp_array(find(tmp_array(:,1) == k),:);
        total_gen_num = total_gen_num + d2(end,end);
    end
    mpdc_10_2c_st_delivery_ratio(ii) = total_receive_num/total_gen_num;
    
    
    %% mpdc sf =10, with 3 channels,with stagger
    file_name = [output_data_folder,'/mpdc_10_3C_st_',int2str(pgi(ii)),'s.txt'];
    [~,~,~,~,scr_id,gen_num,grade,latency,~]=textread(file_name,data_format);
    tmp_array = [scr_id,gen_num];
    % get the min node id and the max node id
    min_id = min(scr_id);
    max_id = max(scr_id);
    total_gen_num = 0;
    total_receive_num = length(gen_num);
    for k = min_id:max_id
        d2= tmp_array(find(tmp_array(:,1) == k),:);
        total_gen_num = total_gen_num + d2(end,end);
    end
    mpdc_10_3c_st_delivery_ratio(ii) = total_receive_num/total_gen_num;
end
disp(dcpf_10_1c_nst_delivery_ratio)
dcpf_10_1c_nst_delivery_ratio
l_dcpf_10_1c_nst = 'r--p';l_mpdc_10_2c_nst = 'm->';
l_mpdc_10_2c_st = 'b-o';l_mpdc_10_3c_st = 'c-^';


linewidth_value=2;
%semilogy
dcpf_10_1c_nst_ratio = plot(pgi,dcpf_10_1c_nst_delivery_ratio,l_dcpf_10_1c_nst,'LineWidth',linewidth_value);
hold on;
mpdc_10_2c_nst_ratio = plot(pgi,mpdc_10_2c_nst_delivery_ratio,l_mpdc_10_2c_nst,'LineWidth',linewidth_value);
mpdc_10_2c_st_ratio = plot(pgi,mpdc_10_2c_st_delivery_ratio,l_mpdc_10_2c_st,'LineWidth',linewidth_value);
mpdc_10_3c_st_ratio = plot(pgi,mpdc_10_3c_st_delivery_ratio,l_mpdc_10_3c_st,'LineWidth',linewidth_value);

hold on;

box on;
xlabel('Packet Generation Interval (s)','fontsize',16); %设置x轴的标题和字体大小
ylabel('Packet Delivery Ratio (s)','fontsize',16); %设置y轴的标题和字体大小

AX = legend([dcpf_10_1c_nst_ratio mpdc_10_2c_nst_ratio mpdc_10_2c_st_ratio mpdc_10_3c_st_ratio], ...
     'DCPF','MPDC (Case 1)','MPDC(Case 2)','MPDC(Case 3)','Best');
 
LEG = findobj(AX,'type','text');

set(LEG,'FontSize',11);%设置legend字体大小
set(gca,'FontSize',16);%设置坐标字体大小
grid on;

