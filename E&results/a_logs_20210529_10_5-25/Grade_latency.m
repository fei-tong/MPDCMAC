close all 
clc


pgi = 10;
g = [1 2 3 4 5 6 7 8 9 10];
dcpf_10_1c_nst_avg_grade_latency = zeros(10,1);
mpdc_10_2c_nst_avg_grade_latency = zeros(10,1);
mpdc_10_2c_st_avg_grade_latency = zeros(10,1);
mpdc_10_3c_st_avg_grade_latency = zeros(10,1);

contikimac_avg_grade_latency = zeros(10,1);
cxmac_avg_grade_latency = zeros(10,1);
nullrdc_avg_grade_latency = zeros(10,1);



output_data_folder = 'R_data';
data_format = '%s %d %d %s %s %s %d %d %d';
for ii = 1:10
    %% dcpf sf =10, with 1 channels
    file_name = [output_data_folder,'/dcpf_10_1C_nst_',int2str(pgi),'s.txt'];
    disp(file_name)
    [~,sink_id,rcv_time,~,~,~,grade,latency,~]=textread(file_name,data_format);
    tmp_array = [grade,latency];
     tmp_array = tmp_array(tmp_array(:,1)== ii,:);
     dcpf_10_1c_nst_avg_grade_latency(ii) = sum(tmp_array(:,2))/length(tmp_array);


    %% mpdc sf =10, with 2 channels,without stagger
    file_name = [output_data_folder,'/mpdc_6_2C_nst_',int2str(pgi),'s.txt'];
    disp(file_name)
    [~,sink_id,rcv_time,~,~,~,grade,latency,~]=textread(file_name,data_format);
    tmp_array = [grade,latency];
     tmp_array = tmp_array(tmp_array(:,1)== ii,:);
     mpdc_10_2c_nst_avg_grade_latency(ii) = sum(tmp_array(:,2))/length(tmp_array);
    %% mpdc sf =10, with 2 channels,with stagger
    file_name = [output_data_folder,'/mpdc_6_2C_st_',int2str(pgi),'s.txt'];
    disp(file_name)
    [~,sink_id,rcv_time,~,~,~,grade,latency,~]=textread(file_name,data_format);
    tmp_array = [grade,latency];
     tmp_array = tmp_array(tmp_array(:,1)== ii,:);
     mpdc_10_2c_st_avg_grade_latency(ii) = sum(tmp_array(:,2))/length(tmp_array);
    
    %% mpdc sf =10, with 3 channels
    file_name = [output_data_folder,'/mpdc_6_3C_st_',int2str(pgi),'s.txt'];
    [~,sink_id,rcv_time,~,~,~,grade,latency,~]=textread(file_name,data_format);
    tmp_array = [grade,latency];
     tmp_array = tmp_array(tmp_array(:,1)== ii,:);
     mpdc_10_3c_st_avg_grade_latency(ii) = sum(tmp_array(:,2))/length(tmp_array);

    
    %% contikimac
    file_name = [output_data_folder,'/contikimac_',int2str(pgi),'s.txt'];
    [~,sink_id,rcv_time,~,~,~,grade,latency,~]=textread(file_name,data_format);
    tmp_array = [grade,latency];
    tmp_array = tmp_array(tmp_array(:,1)== ii,:);
    disp([sum(tmp_array(:,2)),length(tmp_array)]);
    if length(tmp_array) ~= 0
         tmp_array = tmp_array(tmp_array(:,1)== ii,:);
          contikimac_avg_grade_latency(ii) = sum(tmp_array(:,2))/length(tmp_array);
    else
         contikimac_avg_grade_latency(ii) = 0;
    end
   

    
    %% cxmac
    file_name = [output_data_folder,'/xmac_',int2str(pgi),'s.txt'];
    [~,sink_id,rcv_time,~,~,~,grade,latency,~]=textread(file_name,data_format);
    tmp_array = [grade,latency];
    if length(tmp_array) ~= 0
         tmp_array = tmp_array(tmp_array(:,1)== ii,:);
         cxmac_avg_grade_latency(ii) = sum(tmp_array(:,2))/length(tmp_array);
    else
         cxmac_avg_grade_latency(ii) = 0;
    end
    
    
    %% nullmac
    file_name = [output_data_folder,'/nullrdc_',int2str(pgi),'s.txt'];
    [~,sink_id,rcv_time,~,~,~,grade,latency,~]=textread(file_name,data_format);
    tmp_array = [grade,latency];
    tmp_array = tmp_array(tmp_array(:,1)== ii,:);
    if length(tmp_array) ~= 0
         tmp_array = tmp_array(tmp_array(:,1)== ii,:);
          nullrdc_avg_grade_latency(ii) = sum(tmp_array(:,2))/length(tmp_array);
    else
         nullrdc_avg_grade_latency(ii) = 0;
    end
    
end
l_mpdc_10_2c_st = 'r-o';l_mpdc_10_3c_st = 'b--^';
l_mpdc_10_2c_nst = 'g->';


l_contikimac = 'c-d'; l_cxmac = 'm:s';l_nullrdc = 'y--p';

linewidth_value=2;
%semilogy
disp(g);
% disp(mpdc_10_2c_nst_avg_grade_latency);
mpdc_10_2c_nst_latency = plot(g,mpdc_10_2c_nst_avg_grade_latency,l_mpdc_10_2c_nst,'LineWidth',linewidth_value);
hold on;
mpdc_10_2c_st_latency = plot(g,mpdc_10_2c_st_avg_grade_latency,l_mpdc_10_2c_st,'LineWidth',linewidth_value);
mpdc_10_3c_st_latency = plot(g,mpdc_10_3c_st_avg_grade_latency,l_mpdc_10_3c_st,'LineWidth',linewidth_value);

hold on;
disp(contikimac_avg_grade_latency);
contiki_latency = plot(g,contikimac_avg_grade_latency,l_contikimac,'LineWidth',linewidth_value);
cxmac_latency = plot(g,cxmac_avg_grade_latency,l_cxmac,'LineWidth',linewidth_value);
nullrdc_latency = plot(g,nullrdc_avg_grade_latency,l_nullrdc,'Color',[1 0.5 0],'LineWidth',linewidth_value);

box on;
xlabel('包生成间隔 (s)','fontsize',16); %设置x轴的标题和字体大小
ylabel('端到端延迟 (s)','fontsize',16); %设置y轴的标题和字体大小

% AX = legend([ mpdc_10_2c_nst_latency mpdc_10_2c_st_latency mpdc_10_3c_st_latency ], ...
%      'MPDC (Case 1)','MPDC(Case 2)','MPDC(Case 3)');

 AX = legend([ mpdc_10_2c_nst_latency mpdc_10_2c_st_latency mpdc_10_3c_st_latency contiki_latency cxmac_latency nullrdc_latency], ...
     'MPDC (Case 1)','MPDC(Case 2)','MPDC(Case 3)','CCP(ContikiMAC)','CCP(X-MAC)','CCP(Full-Active Radio)','Best');

 set(AX,'box','off')

LEG = findobj(AX,'type','text');

set(LEG,'FontSize',11);%设置legend字体大小
set(gca,'FontSize',16);%设置坐标字体大小
grid on;


