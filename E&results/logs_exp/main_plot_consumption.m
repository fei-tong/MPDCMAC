close all 
file_num = [1 2 3 4 5];
pgi = [10 15 20 25 30];

dcpf_10_1c_nst_consumption_array = zeros(5,1);
mpdc_10_2c_st_consumption_array = zeros(5,1);
mpdc_10_2c_nst_consumption_array = zeros(5,1);
mpdc_10_3c_st_consumption_array = zeros(5,1);

contikimac_consumption_array = zeros(5,1);
cxmac_consumption_array = zeros(5,1);
nullrdc_consumption_array = zeros(5,1);
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
    id_index=((ID~=exc_id_1) & (ID~=exc_id_2));
    total_consumption = sum((control(id_index)+data(id_index)+idle(id_index)))/length(total(id_index));
%      total_consumption = control(end)+data(end);
    mpdc_10_2c_st_consumption_array(ii) = total_consumption/mpdc_10_2c_st_rcvd_pkt_num(ii);
    
    %% mpdc sf =10, with 2 channels, without stagger
    r_file_name = [output_data_folder,'/mpdc_10_2C_nst_',int2str(pgi(ii)),'s.txt'];
    [~,~,~,~,~,~,~,latency,~]=textread(r_file_name,data_format2);
    mpdc_10_2c_nst_rcvd_pkt_num(ii) = length(latency);
    
    file_name = [data_folder,'/mpdc_10_2C_nst_',int2str(pgi(ii)),'s.txt'];
    [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
    mpdc2_array = [ID,control,data,idle,total];
    id_index=((ID~=exc_id_2));
     total_consumption = sum((control(id_index)+data(id_index)))/length(total(id_index));
%      total_consumption = control(end)+data(end);
     mpdc_10_2c_nst_consumption_array(ii) = total_consumption/mpdc_10_2c_nst_rcvd_pkt_num(ii);
     
    %% mpdc sf =10, with 3 channels
    r_file_name = [output_data_folder,'/mpdc_10_3C_st_',int2str(pgi(ii)),'s.txt'];
    [~,~,~,~,~,~,~,latency,~]=textread(r_file_name,data_format2);
    mpdc_10_3c_st_rcvd_pkt_num(ii) = length(latency);
    
    file_name = [data_folder,'/mpdc_10_3C_st_',int2str(pgi(ii)),'s.txt'];
    [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
    id_index=((ID~=exc_id_1) & (ID~=exc_id_2) &(ID~=exc_id_3));
    total_consumption = sum((control(id_index)+data(id_index)+idle(id_index)))/length(total(id_index));
    %total_consumption = control(end)+data(end);
    mpdc_10_3c_st_consumption_array(ii) = total_consumption/mpdc_10_3c_st_rcvd_pkt_num(ii);
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %% contiki
    r_file_name = [output_data_folder,'/contikimac_',int2str(pgi(ii)),'s.txt'];
    [~,~,~,~,~,~,~,latency,~]=textread(r_file_name,data_format2);
    contikimac_rcvd_pkt_num(ii) = sum(latency~=0);
    
    file_name = [data_folder,'/contikimac_',int2str(pgi(ii)),'s.txt'];
    [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
    id_index=(ID~=exc_id_1);
%     total_consumption = data(end);
     total_consumption = sum((control(id_index)+data(id_index)+idle(id_index)))/length(total(id_index));
    contikimac_consumption_array(ii) = total_consumption/contikimac_rcvd_pkt_num(ii);  
   
    %% cxmac
   r_file_name = [output_data_folder,'/xmac_',int2str(pgi(ii)),'s.txt'];
    [~,~,~,~,~,~,~,latency,~]=textread(r_file_name,data_format2);
    
    cxmac_rcvd_pkt_num(ii) = length(latency);
    
    file_name = [data_folder,'/xmac_',int2str(pgi(ii)),'s.txt'];
    [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
    cxmac_array = [ID,control,data,idle,total];
    id_index=(ID~=exc_id_1);
     total_consumption = data(end);
%     total_consumption = sum((control(id_index)+data(id_index)+idle(id_index)))/length(total(id_index));
    cxmac_consumption_array(ii) = total_consumption/cxmac_rcvd_pkt_num(ii);
     
    %% nullrdc
    r_file_name = [output_data_folder,'/nullrdc_',int2str(pgi(ii)),'s.txt'];
    [~,~,~,~,~,~,~,latency,~]=textread(r_file_name,data_format2);
    nullrdc_rcvd_pkt_num(ii) = length(latency);
    
    file_name = [data_folder,'/nullrdc_',int2str(pgi(ii)),'s.txt'];
    [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
id_index=(ID~=exc_id_1);
%       total_consumption = data(end);
    total_consumption = sum((control(id_index)+data(id_index)+idle(id_index)))/length(total(id_index));
    nullrdc_consumption_array(ii) = total_consumption/nullrdc_rcvd_pkt_num(ii);
   
     
    jj=jj+1;
end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
linewidth_value = 2;
l_mpdc_10_2c_st = 'r-o';l_mpdc_10_3c_st = 'b--^';
l_mpdc_10_2c_nst = 'g->';


l_contikimac = 'c-d'; l_cxmac = 'm:s';l_nullrdc = 'k--p';

mpdc_10_2c_st_com = semilogy(pgi,mpdc_10_2c_st_consumption_array/1000,l_mpdc_10_2c_st,'LineWidth',linewidth_value);

hold on;
mpdc_10_2c_nst_com = plot(pgi,mpdc_10_2c_nst_consumption_array/1000,l_mpdc_10_2c_nst,'LineWidth',linewidth_value);
mpdc_10_3c_st_com = plot(pgi,mpdc_10_3c_st_consumption_array/1000,l_mpdc_10_3c_st,'LineWidth',linewidth_value);
disp(mpdc_10_2c_nst_consumption_array)
hold on;
contikimac_com = plot(pgi,contikimac_consumption_array/1000,l_contikimac,'LineWidth',linewidth_value);
 cxmac_com = plot(pgi,cxmac_consumption_array/1000,l_cxmac,'LineWidth',linewidth_value);
nullrdc_com = plot(pgi,nullrdc_consumption_array/1000,l_nullrdc,'Color',[1 0.5 0],'LineWidth',linewidth_value);
disp(cxmac_consumption_array)
% 
%axis([10 30 1 7]);
box on;
xlabel('包生成间隔 (s)','fontsize',16); %设置x轴的标题和字体大小
ylabel('包平均能耗 (mV)','fontsize',16);
AX = legend([ mpdc_10_2c_nst_com mpdc_10_2c_st_com mpdc_10_3c_st_com contikimac_com cxmac_com nullrdc_com],...
   'MPDC (Case 1)','MPDC (Case 2)','MPDC (Case 3)','CCP (ContikiMAC)','CCP (X-MAC)','CCP (Full-Active Radio)','Best');

% AX = legend([ mpdc_10_2c_nst_com mpdc_10_2c_st_com mpdc_10_3c_st_com contikimac_com cxmac_com ],...
%    'MPDC (Case 1)','MPDC(Case 2)','MPDC(Case 3)','CCP(ContikiMAC)','CCP(X-MAC)','Best');

% set(AX,'box','off')
LEG = findobj(AX,'type','text');

set(LEG,'FontSize',11);%设置legend字体大小
set(gca,'FontSize',16);%设置坐标字体大小
grid on;