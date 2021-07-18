close all 
clc

file_num = [1 2 3 4 5];
pgi = [5 10 15 20 25];

dcpf_10_1c_nst_delivery_ratio = zeros(5,1);
mpdc_10_2c_nst_delivery_ratio = zeros(5,1);
mpdc_10_2c_st_delivery_ratio = zeros(5,1);
mpdc_10_3c_st_delivery_ratio = zeros(5,1);

% contikimac_delivery_ratio = zeros(5,1);
% cxmac_delivery_ratio = zeros(5,1);
% nullrdc_delivery_ratio = zeros(5,1);


output_data_folder = 'R_data';
data_format = '%s %d %d %s %f %d %d %d %d';
data_folder = 'P_data';
data_format2='%s %d %s %d %d %d %d'; % original format: '%s %s %d %s %s %s %d %d %d %d'


    
for ii = file_num
%     %% dcpf,sf = 10 
%     file_name = [output_data_folder,'/dcpf_10_1C_nst_',int2str(pgi(ii)),'s.txt'];
%     [~,~,~,~,scr_id,gen_num,grade,latency,~]=textread(file_name,data_format);
%     tmp_array = [scr_id,gen_num];
%     % get the min node id and the max node id
%     min_id = min(scr_id);
%     max_id = max(scr_id);
%     total_gen_num = 0;
%     total_receive_num = length(gen_num);
% %     disp([min_id,max_id]);
%     for k = min_id:max_id
%         d2= tmp_array(find(tmp_array(:,1) == k),:);
%         if length(d2)~= 0 
%             total_gen_num = total_gen_num + d2(end,end);
%         else
%          %   disp(k);
%         end
%     end
%    % disp([total_receive_num,total_gen_num]);
%     dcpf_10_1c_nst_delivery_ratio(ii) = total_receive_num/total_gen_num;
    
    %% mpdc sf =10, with 2 channels,without stagger
    file_name = [output_data_folder,'/mpdc_10_2C_nst_',int2str(pgi(ii)),'s.txt'];
    [~,~,~,~,scr_id,gen_num,grade,latency,~]=textread(file_name,data_format);
    tmp_array = [scr_id,gen_num];
   
    % get the min node id and the max node id
    min_id = min(scr_id);
    max_id = max(scr_id);
    disp([min_id,max_id]);
    total_gen_num = 0;
    total_receive_num = length(gen_num);
    for k = min_id:max_id
        d2= tmp_array(find(tmp_array(:,1) == k),:);
         if length(d2)~= 0 
            total_gen_num = total_gen_num + d2(end,end);
         else 
%              disp(k);
         end
    end
    mpdc_10_2c_nst_delivery_ratio(ii) = total_receive_num/total_gen_num;
    
   %% mpdc sf =10, with 2 channels,with stagger
    file_name = [output_data_folder,'/mpdc_10_2C_st_',int2str(pgi(ii)),'s.txt'];
    [~,~,~,~,scr_id,gen_num,grade,latency,~]=textread(file_name,data_format);
    tmp_array = [scr_id,gen_num];
    % get the min node id and the max node id
    min_id = min(scr_id);
    max_id = max(scr_id);
    disp([min_id,max_id]);
    t = tmp_array;
    total_gen_num = 0;
    total_receive_num = length(gen_num);
    for k = min_id:max_id
        d2= tmp_array(find(tmp_array(:,1) == k),:);
         if length(d2)~= 0 
            total_gen_num = total_gen_num + d2(end,end);
         else
             disp(k);
        end
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
         if length(d2)~= 0 
            total_gen_num = total_gen_num + d2(end,end);
         else
          %   disp(k);
        end
    end
    mpdc_10_3c_st_delivery_ratio(ii) = total_receive_num/total_gen_num;
    
    
    %% contikimac
    file_name = [output_data_folder,'/contikimac_',int2str(pgi(ii)),'s.txt'];
     [~,~,~,~,scr_id,gen_num,grade,latency,~]=textread(file_name,data_format);
    tmp_array = [scr_id,gen_num];
    % get the min node id and the max node id
    min_id = min(scr_id);
    max_id = max(scr_id);
    total_gen_num = 0;
    total_receive_num = length(gen_num);
    for k = min_id:max_id
        d2= tmp_array(find(tmp_array(:,1) == k),:);
        if length(d2) == 0
            disp([2,k]);
        else
            total_gen_num = total_gen_num + d2(end,end) + 1;
        end
    end
    contikimac_delivery_ratio(ii) = total_receive_num/total_gen_num;

%     file_name = [data_folder,'/contikimac_',int2str(pgi(ii)),'s.txt'];
%     [~,ID,~,~,~,~,~]=textread(file_name,data_format2);
%     
%     total_receive_num = length(latency);
%     total_gen_num = length(ID);
%     contikimac_delivery_ratio(ii) = total_receive_num/total_gen_num;
    
    
    %% cxmac
    file_name = [output_data_folder,'/xmac_',int2str(pgi(ii)),'s.txt'];
    [~,~,~,~,scr_id,gen_num,grade,latency,~]=textread(file_name,data_format);
        tmp_array = [scr_id,gen_num];
    % get the min node id and the max node id
    min_id = min(scr_id);
    max_id = max(scr_id);
    total_gen_num = 0;
    total_receive_num = length(gen_num);
    for k = min_id:max_id
        d2= tmp_array(find(tmp_array(:,1) == k),:);
        if length(d2) == 0
            disp([3,k]);
        else
            total_gen_num = total_gen_num + d2(end,end) + 1;
        end
    end
    cxmac_delivery_ratio(ii) = total_receive_num/total_gen_num;
    
% 
%     file_name = [data_folder,'/xmac_',int2str(pgi(ii)),'s.txt'];
%     [~,ID,~,~,~,~,~]=textread(file_name,data_format2);
%     
%     total_receive_num = length(latency);
%     total_gen_num = length(ID);
%     cxmac_delivery_ratio(ii) = total_receive_num/total_gen_num;   
    
    %% nullrdc
    file_name = [output_data_folder,'/nullrdc_',int2str(pgi(ii)),'s.txt'];
     [~,~,~,~,scr_id,gen_num,grade,latency,~]=textread(file_name,data_format);
    tmp_array = [scr_id,gen_num];
    % get the min node id and the max node id
    min_id = min(scr_id);
    max_id = max(scr_id);
    total_gen_num = 0;
    total_receive_num = length(gen_num);
    for k = min_id:max_id
        d2= tmp_array(find(tmp_array(:,1) == k),:);
        total_gen_num = total_gen_num + d2(end,end) + 1;
    end
    nullrdc_delivery_ratio(ii) = total_receive_num/total_gen_num;
    
% 
%     file_name = [data_folder,'/nullrdc_',int2str(pgi(ii)),'s.txt'];
%     [~,ID,~,~,~,~,~]=textread(file_name,data_format2);
%     
%     total_receive_num = length(latency);
%     total_gen_num = length(ID);
%     nullrdc_delivery_ratio(ii) = total_receive_num/total_gen_num;
    
end
% disp(nullrdc_delivery_ratio)
linewidth_value = 2;
l_mpdc_10_2c_st = 'r-o';l_mpdc_10_3c_st = 'b--^';
l_mpdc_10_2c_nst = 'g->';


l_contikimac = 'c-d'; l_cxmac = 'm:s';l_nullrdc = 'y--p';
%semilogy
mpdc_10_2c_nst_ratio = plot(pgi,mpdc_10_2c_nst_delivery_ratio*100,l_mpdc_10_2c_nst,'LineWidth',linewidth_value);
hold on;

mpdc_10_2c_st_ratio = plot(pgi,mpdc_10_2c_st_delivery_ratio*100,l_mpdc_10_2c_st,'LineWidth',linewidth_value);
mpdc_10_3c_st_ratio = plot(pgi,mpdc_10_3c_st_delivery_ratio*100,l_mpdc_10_3c_st,'LineWidth',linewidth_value);

hold on;
contikimac_ratio = plot(pgi,contikimac_delivery_ratio*100,l_contikimac,'LineWidth',linewidth_value);
cxmac_ratio = plot(pgi,cxmac_delivery_ratio*100,l_cxmac,'LineWidth',linewidth_value);
nullrdc_ratio = plot(pgi,nullrdc_delivery_ratio*100,l_nullrdc,'Color',[1 0.5 0],'LineWidth',linewidth_value);

box on;
xlabel('Packet Generation Interval (s)','fontsize',16); %设置x轴的标题和字体大小
ylabel('Packet Delivery Ratio(%)','fontsize',16); %设置y轴的标题和字体大小
% 
AX = legend([ mpdc_10_2c_nst_ratio mpdc_10_2c_st_ratio mpdc_10_3c_st_ratio contikimac_ratio cxmac_ratio nullrdc_ratio], ...
     'MPDC (Case 1)','MPDC(Case 2)','MPDC(Case 3)','CCP(ContikiMAC)','CCP(X-MAC)','CCP(Full-Active Radio)','Best');

%  AX = legend([ mpdc_10_2c_nst_ratio mpdc_10_2c_st_ratio mpdc_10_3c_st_ratio], ...
%      'MPDC (Case 1)','MPDC(Case 2)','MPDC(Case 3)','Best');

 % set(AX,'box','off')
LEG = findobj(AX,'type','text');
disp(nullrdc_delivery_ratio)
set(LEG,'FontSize',11);%设置legend字体大小
set(gca,'FontSize',16);%设置坐标字体大小
grid on;

