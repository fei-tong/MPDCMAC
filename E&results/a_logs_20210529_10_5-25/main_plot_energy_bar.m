close all 
file_num = [1 2 3 4 5];
pgi = [5 10 15 20 25];

dcpf_10_1c_nst_array = zeros(5,1);
mpdc_10_2c_st_array = zeros(5,1);
mpdc_10_2c_nst_array = zeros(5,1);
mpdc_10_3c_st_array = zeros(5,1);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
ccp_contiki_array = zeros(5,1);
ccp_cxmac_array = zeros(5,1);
ccp_nullrdc_array = zeros(5,1);


data_folder = 'P_data';
data_format='%s %d %s %d %d %d %d'; % original format: '%s %s %d %s %s %s %d %d %d %d'
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
jj = 1;
exc_id_1=1; % excluded id
exc_id_2=2;
exc_id_3 = 3
for ii = file_num
    %% mpdc sf =10, with 2 channels,with stagger
    file_name = [data_folder,'/mpdc_10_2C_st_',int2str(pgi(ii)),'s.txt'];
    disp(file_name)
    [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
    id_index=((ID~=exc_id_1) & (ID~=exc_id_2));% They are sky motes, they will not be conunted
    mpdc_10_2c_st_array(jj) = sum((control(id_index)+data(id_index)+idle(id_index))./total(id_index))/length(total(id_index));
    
    %% mpdc sf =10, with 2 channels, without stagger
    file_name = [data_folder,'/mpdc_10_2C_nst_',int2str(pgi(ii)),'s.txt'];
    [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
    id_index=((ID~=exc_id_1) & (ID~=exc_id_2));% They are sky motes, they will not be conunted
    mpdc_10_2c_nst_array(jj) = sum((control(id_index)+data(id_index)+idle(id_index))./total(id_index))/length(total(id_index));
   
    %% mpdc sf =10, with 3 channels
    file_name = [data_folder,'/mpdc_10_3C_st_',int2str(pgi(ii)),'s.txt'];
    [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
    id_index=((ID~=exc_id_1) & (ID~=exc_id_2) &(ID~=exc_id_3));
    mpdc_10_3c_st_array(jj) = sum((control(id_index)+data(id_index)+idle(id_index))./total(id_index))/length(total(id_index));

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%     %% pdc sf =10, with 1 channel
%     file_name = [data_folder,'/dcpf_6_1C_nst_',int2str(pgi(ii)),'s.txt'];
%     [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
%     id_index= (ID~=exc_id_1);
%     dcpf_10_1c_nst_array(jj) = sum((control(id_index)+data(id_index)+idle(id_index))./total(id_index))/length(total(id_index));
%     
    %% contikimac
    file_name = [data_folder,'/contikimac_',int2str(pgi(ii)),'s.txt'];
    [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
    id_index= (ID~=exc_id_1);
    ccp_contiki_array(jj) = sum((control(id_index)+data(id_index)+idle(id_index))./total(id_index))/length(total(id_index));
    %% cxmac
     file_name = [data_folder,'/xmac_',int2str(pgi(ii)),'s.txt'];
     [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
     id_index= (ID~=exc_id_1);
     ccp_cxmac_array(jj) = sum((control(id_index)+data(id_index)+idle(id_index))./total(id_index))/length(total(id_index));
    %% nullrdc
    file_name = [data_folder,'/nullrdc_',int2str(pgi(ii)),'s.txt'];
    [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
    id_index= (ID~=exc_id_1);
    ccp_nullrdc_array(jj) = sum((control(id_index)+data(id_index)+idle(id_index))./total(id_index))/length(total(id_index));
    
    
    jj=jj+1;
end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
linewidth_value = 2;
l_mpdc_10_2c_st = 'r-o';l_mpdc_10_3c_st = 'b--^';
l_mpdc_10_2c_nst = 'g->';


l_contikimac = 'c-d'; l_cxmac = 'm:s';l_nullrdc = 'y--p';

mpdc_10_2c_st_dc = semilogy(pgi,mpdc_10_2c_st_array*100,l_mpdc_10_2c_st,'LineWidth',linewidth_value);
hold on;
mpdc_10_2c_nst_dc = semilogy(pgi,mpdc_10_2c_nst_array*100,l_mpdc_10_2c_nst,'LineWidth',linewidth_value);
mpdc_10_3c_st_dc = semilogy(pgi,mpdc_10_3c_st_array*100,l_mpdc_10_3c_st,'LineWidth',linewidth_value);


contiki_dc = semilogy(pgi,ccp_contiki_array*100,l_contikimac,'LineWidth',linewidth_value);
cxmac_dc = semilogy(pgi,ccp_cxmac_array*100,l_cxmac,'LineWidth',linewidth_value);
% nullrdc_dc = semilogy(pgi,ccp_nullrdc_array*100,l_nullrdc,'Color',[1 0.5 0],'LineWidth',linewidth_value);

% 

box on;
xlabel('包生成间隔PGI(s)','fontsize',16); %设置x轴的标题和字体大小
ylabel('占空比(%)','fontsize',16);


%  AX = legend([mpdc_10_2c_nst_dc mpdc_10_2c_st_dc mpdc_10_3c_st_dc contiki_dc cxmac_dc nullrdc_dc],...
%      'MPDC (Case 1)','MPDC(Case 2)','MPDC(Case 3)','CCP(ContikiMAC)','CCP(X-MAC)','CCP(Full-Active Radio)','NorthEast');
 AX = legend([mpdc_10_2c_nst_dc mpdc_10_2c_st_dc mpdc_10_3c_st_dc contiki_dc cxmac_dc ],...
     'MPDC (Case 1)','MPDC(Case 2)','MPDC(Case 3)','CCP(ContikiMAC)','CCP(X-MAC)','NorthEast');

 %  AX = legend([mpdc_10_2c_nst_dc mpdc_10_2c_st_dc mpdc_10_3c_st_dc ],...
%      'MPDC (Case 1)','MPDC(Case 2)','MPDC(Case 3)','NorthEast');

 
 set(AX,'box','off')
LEG = findobj(AX,'type','text');

set(LEG,'FontSize',11);%设置legend字体大小
set(gca,'FontSize',16);%设置坐标字体大小
grid on;