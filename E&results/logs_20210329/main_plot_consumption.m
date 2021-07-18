close all 
file_num = [1 2 3 4 5];
pgi = [10 15 20 25 30];

dcpf_10_1c_nst_consumption_array = zeros(5,1);
mpdc_10_2c_st_consumption_array = zeros(5,1);
mpdc_10_2c_nst_consumption_array = zeros(5,1);
mpdc_10_3c_st_consumption_array = zeros(5,1);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
dcpf_10_1c_nst_rcvd_pkt_num = zeros(5,1);
mpdc_10_2c_nst_rcvd_pkt_num = zeros(5,1);
mpdc_10_2c_st_rcvd_pkt_num = zeros(5,1);
mpdc_10_3c_st_rcvd_pkt_num = zeros(5,1);


data_folder = 'P_data';
data_format='%s %d %s %d %d %d %d'; % original format: '%s %s %d %s %s %s %d %d %d %d'
output_data_folder = 'R_data';
data_format2 = '%s %s %d %s %f %d %d %d %d';
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
jj = 1;
exc_id_1=1; % excluded id
exc_id_2=2;
exc_id_3 = 3
for ii = file_num
    %% mpdc sf =10, with 2 channels,with stagger
    r_file_name = [output_data_folder,'/mpdc_10_2C_st_',int2str(pgi(ii)),'s.txt'];
    [~,~,~,~,~,~,~,latency,~]=textread(r_file_name,data_format2);
    mpdc_10_2c_st_rcvd_pkt_num(ii) = length(latency);
    
    file_name = [data_folder,'/mpdc_10_2C_st_',int2str(pgi(ii)),'s.txt'];
    [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
    tmp_array = sortrows([ID,total],1);
    tmp_array = unique(tmp_array,'rows');

    d2 = tmp_array(find(tmp_array(:,1) == 1),:);
    total_consumption =  d2(end,end);
    mpdc_10_2c_st_consumption_array(ii) = total_consumption/mpdc_10_2c_st_rcvd_pkt_num(ii);

    %% mpdc sf =10, with 2 channels, without stagger
    r_file_name = [output_data_folder,'/mpdc_10_2C_nst_',int2str(pgi(ii)),'s.txt'];
    [~,~,~,~,~,~,~,latency,~]=textread(r_file_name,data_format2);
    mpdc_10_2c_nst_rcvd_pkt_num(ii) = length(latency);
    
    file_name = [data_folder,'/mpdc_10_2C_nst_',int2str(pgi(ii)),'s.txt'];
    [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
    tmp_array = sortrows([ID,total],1);
    tmp_array = unique(tmp_array,'rows');
    d2 = tmp_array(find(tmp_array(:,1) == 1),:);
    total_consumption =  d2(end,end);
    mpdc_10_2c_nst_consumption_array(ii) = total_consumption/mpdc_10_2c_nst_rcvd_pkt_num(ii);
    %% mpdc sf =10, with 3 channels
    r_file_name = [output_data_folder,'/mpdc_10_3C_st_',int2str(pgi(ii)),'s.txt'];
    [~,~,~,~,~,~,~,latency,~]=textread(r_file_name,data_format2);
    mpdc_10_3c_st_rcvd_pkt_num(ii) = length(latency);
    
    file_name = [data_folder,'/mpdc_10_3C_st_',int2str(pgi(ii)),'s.txt'];
    [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
    tmp_array = sortrows([ID,total],1);
    tmp_array = unique(tmp_array,'rows');
    d2 = tmp_array(find(tmp_array(:,1) == 1),:);
    total_consumption =  d2(end,end);
    mpdc_10_3c_st_consumption_array(ii) = total_consumption/mpdc_10_3c_st_rcvd_pkt_num(ii);
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %% pdc sf =10, with 1 channel
    r_file_name = [output_data_folder,'/dcpf_10_1C_nst_',int2str(pgi(ii)),'s.txt'];
    [~,~,~,~,~,~,~,latency,~]=textread(r_file_name,data_format2);
    dcpf_10_1c_nst_rcvd_pkt_num(ii) = length(latency);
    
    file_name = [data_folder,'/dcpf_10_1C_nst_',int2str(pgi(ii)),'s.txt'];
    [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
    tmp_array = sortrows([ID,total],1);
    tmp_array = unique(tmp_array,'rows');
    d2 = tmp_array(find(tmp_array(:,1) == 1),:);
    total_consumption =  d2(end,end);
    dcpf_10_1c_nst_consumption_array(ii) = total_consumption/dcpf_10_1c_nst_rcvd_pkt_num(ii);

    jj=jj+1;
end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
linewidth_value = 2;
l_mpdc_10_2c_st = 'b-o';l_mpdc_10_3c_st = 'c-^';
l_mpdc_10_2c_nst = 'm->';l_dcpf_10_1c_nst = 'r--p';

mpdc_10_2c_st_com = plot(pgi,mpdc_10_2c_st_consumption_array/10000,l_mpdc_10_2c_st,'LineWidth',linewidth_value);
hold on;
mpdc_10_2c_nst_com = plot(pgi,mpdc_10_2c_nst_consumption_array/10000,l_mpdc_10_2c_nst,'LineWidth',linewidth_value);
mpdc_10_3c_st_com = plot(pgi,mpdc_10_3c_st_consumption_array/10000,l_mpdc_10_3c_st,'LineWidth',linewidth_value);

dcpf_10_1c_nst_com = plot(pgi,dcpf_10_1c_nst_consumption_array/10000,l_dcpf_10_1c_nst,'LineWidth',linewidth_value);

% 
%axis([10 30 1 7]);
box on;
xlabel('Packet Generation Interval (s)','fontsize',16); %设置x轴的标题和字体大小
ylabel('Power Consumption/Packet','fontsize',16);
AX = legend([dcpf_10_1c_nst_com mpdc_10_2c_nst_com mpdc_10_2c_st_com mpdc_10_3c_st_com],...
    'DCPF','MPDC (Case 1)','MPDC(Case 2)','MPDC(Case 3)','Best');
LEG = findobj(AX,'type','text');

set(LEG,'FontSize',11);%设置legend字体大小
set(gca,'FontSize',16);%设置坐标字体大小
grid on;