close all 
clc

file_num = [1 2 3 4 5];
pgi = [5 10 15 20 25];

dcpf_10_1c_nst_avg_hop_latency = zeros(5,1);
mpdc_10_2c_nst_avg_hop_latency = zeros(5,1);
mpdc_10_2c_st_avg_hop_latency = zeros(5,1);
mpdc_10_3c_st_avg_hop_latency = zeros(5,1);

contikimac_avg_hop_latency = zeros(5,1);
cxmac_avg_hop_latency = zeros(5,1);
nullrdc_avg_hop_latency = zeros(5,1);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
dcpf_10_1c_nst_avg_delivery  = zeros(5,1);
mpdc_10_2c_nst_avg_delivery  = zeros(5,1);
mpdc_10_2c_st_avg_delivery  = zeros(5,1);
mpdc_10_3c_st_avg_delivery  = zeros(5,1);

contikimac_avg_delivery = zeros(5,1);
cxmac_avg_delivery= zeros(5,1);
nullrdc_avg_delivery = zeros(5,1);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
dcpf_10_1c_nst_rcvd_pkt_num = zeros(5,1);
mpdc_10_2c_nst_rcvd_pkt_num = zeros(5,1);
mpdc_10_2c_st_rcvd_pkt_num = zeros(5,1);
mpdc_10_3c_st_rcvd_pkt_num = zeros(5,1);


contikimac_rcvd_pkt_num = zeros(5,1);
cxmac_rcvd_pkt_num= zeros(5,1);
nullrdc_rcvd_pkt_num = zeros(5,1);

output_data_folder = 'R_data';
data_format = '%s %d %d %s %s %s %d %d %d';
for ii = file_num
%     %% dcpf sf =10, with 1 channels
%     file_name = [output_data_folder,'/dcpf_6_1C_nst_',int2str(pgi(ii)),'s.txt'];
%     disp(file_name)
%     [~,sink_id,rcv_time,~,~,~,grade,latency,~]=textread(file_name,data_format);
%     dcpf_10_1c_nst_avg_hop_latency(ii) = sum(latency./grade)/length(latency)./128;
%  %   pdc_18_nfa_rcvd_pkt_num(ii) = length(latency);
%     
%     data_matrix = [sink_id,rcv_time,grade,latency];
%     [sorted_matrix,I] = sortrows(data_matrix,1);
%     rcv_time = sorted_matrix(:,2);
%     %get the packets received by sink 1
%     pkts_sink_1 = sum(sorted_matrix(:,1)==1);
%     rcv_time=rcv_time./128;
%     total_pkt = length(rcv_time);% the packets received by all sink nodes
%     total_time = 0;
%     for j=2:1:pkts_sink_1
%         total_time = total_time + rcv_time(j)-rcv_time(j-1);
%     end
%     dcpf_10_1c_nst_avg_delivery(ii) = total_pkt/total_time;
%     
%     
    %% mpdc sf =10, with 2 channels,without stagger
    file_name = [output_data_folder,'/mpdc_10_2C_nst_',int2str(pgi(ii)),'s.txt'];
    disp(file_name)
    [~,sink_id,rcv_time,~,~,~,grade,latency,~]=textread(file_name,data_format);
    mpdc_10_2c_nst_avg_hop_latency(ii) = sum(latency./grade)/length(latency)./128;
  %  mpdc_10_2c__rcvd_pkt_num(ii) = length(latency);
    data_matrix = [sink_id,rcv_time,grade,latency];
    [sorted_matrix,I] = sortrows(data_matrix,1);
    rcv_time = sorted_matrix(:,2);
    %get the packets received by sink 1
    pkts_sink_1 = sum(sorted_matrix(:,1)==1);
    rcv_time=rcv_time./128;
    total_pkt = length(rcv_time);% the packets received by all sink nodes
    total_time = 0;
    for j=2:1:pkts_sink_1
        total_time = total_time + rcv_time(j)-rcv_time(j-1);
    end
    mpdc_10_2c_nst_avg_delivery(ii) = total_pkt/total_time;
    %% mpdc sf =10, with 2 channels,with stagger
    file_name = [output_data_folder,'/mpdc_10_2C_st_',int2str(pgi(ii)),'s.txt'];
    disp(file_name)
    [~,sink_id,rcv_time,~,~,~,grade,latency,~]=textread(file_name,data_format);
    mpdc_10_2c_st_avg_hop_latency(ii) = sum(latency./grade)/length(latency)./128;
  %  mpdc_10_2c__rcvd_pkt_num(ii) = length(latency);
    data_matrix = [sink_id,rcv_time,grade,latency];
    [sorted_matrix,I] = sortrows(data_matrix,1);
    rcv_time = sorted_matrix(:,2);
    %get the packets received by sink 1
    pkts_sink_1 = sum(sorted_matrix(:,1)==1);
    rcv_time=rcv_time./128;
    total_pkt = length(rcv_time);% the packets received by all sink nodes
    total_time = 0;
    for j=2:1:pkts_sink_1
        total_time = total_time + rcv_time(j)-rcv_time(j-1);
    end
    mpdc_10_2c_st_avg_delivery(ii) = total_pkt/total_time;
    
    %% mpdc sf =10, with 3 channels
    file_name = [output_data_folder,'/mpdc_10_3C_st_',int2str(pgi(ii)),'s.txt'];
    [~,sink_id,rcv_time,~,~,~,grade,latency,~]=textread(file_name,data_format);

    mpdc_10_3c_st_avg_hop_latency(ii) = sum(latency./grade)/length(latency)./128;
   % adc_18_nfa_rcvd_pkt_num(ii) = length(latency);
    
    data_matrix = [sink_id,rcv_time,grade,latency];
    [sorted_matrix,I] = sortrows(data_matrix,1);
    rcv_time = sorted_matrix(:,2);
    %get the packets received by sink 1
    pkts_sink_1 = sum(sorted_matrix(:,1)==1);
    rcv_time=rcv_time./128;
    total_pkt = length(rcv_time);% the packets received by all sink nodes
    total_time = 0;
    for j=2:1:pkts_sink_1
        total_time = total_time + rcv_time(j)-rcv_time(j-1);
    end
    mpdc_10_3c_st_avg_delivery(ii) = total_pkt/total_time;
    
    %% contikimac
    file_name = [output_data_folder,'/contikimac_',int2str(pgi(ii)),'s.txt'];
    [~,sink_id,rcv_time,~,~,~,grade,latency,~]=textread(file_name,data_format);
    
    contikimac_avg_hop_latency(ii) = sum(latency./grade)/length(latency)./128;
   % adc_18_nfa_rcvd_pkt_num(ii) = length(latency);
    rcv_time=rcv_time./128;
    total_pkt = 0;
    total_time = 0;
    for j=2:1:length(rcv_time)
        total_pkt = total_pkt + 1;
        total_time = total_time + rcv_time(j)-rcv_time(j-1);
    end
    contikimac_avg_delivery(ii) = total_pkt/total_time;
    
    %% cxmac
    file_name = [output_data_folder,'/xmac_',int2str(pgi(ii)),'s.txt'];
    [~,sink_id,rcv_time,~,~,~,grade,latency,~]=textread(file_name,data_format);
    tmp_array2 = [sink_id,rcv_time,grade,latency];
    pos = find(tmp_array2(:,4) == 0);
    tmp_array2(pos, :) = [];
    rcv_time = tmp_array2(:,2);
    grade = tmp_array2(:,3);
    latency = tmp_array2(:,4);
    disp(latency./grade)
    cxmac_avg_hop_latency(ii) = sum(latency./grade)/length(latency)./128;
   % adc_18_nfa_rcvd_pkt_num(ii) = length(latency);
    
    rcv_time=rcv_time./128;
    total_pkt = 0;
    total_time = 0;
    for j=2:1:length(rcv_time)
        total_pkt = total_pkt + 1;
        total_time = total_time + rcv_time(j)-rcv_time(j-1);
    end
    
    disp([total_pkt,total_time,total_pkt/total_time])
    
    cxmac_avg_delivery(ii) = total_pkt/total_time;
    
    %% nullmac
    file_name = [output_data_folder,'/nullrdc_',int2str(pgi(ii)),'s.txt'];
    [~,sink_id,rcv_time,~,~,~,grade,latency,~]=textread(file_name,data_format);

    nullrdc_avg_hop_latency(ii) = sum(latency./grade)/length(latency)./128;
   % adc_18_nfa_rcvd_pkt_num(ii) = length(latency);
    
    rcv_time=rcv_time./128;
    total_pkt = 0;
    total_time = 0;
    for j=2:1:length(rcv_time)
        total_pkt = total_pkt + 1;
        total_time = total_time + rcv_time(j)-rcv_time(j-1);
    end
    
    disp([total_pkt,total_time,total_pkt/total_time])
    
    nullrdc_avg_delivery(ii) = total_pkt/total_time;
    
    
end
l_mpdc_10_2c_st = 'r-o';l_mpdc_10_3c_st = 'b--^';
l_mpdc_10_2c_nst = 'g->';


l_contikimac = 'c-d'; l_cxmac = 'm:s';l_nullrdc = 'y--p';

linewidth_value=2;
%semilogy
mpdc_10_2c_nst_latency = plot(pgi,mpdc_10_2c_nst_avg_hop_latency,l_mpdc_10_2c_nst,'LineWidth',linewidth_value);
hold on;
mpdc_10_2c_st_latency = plot(pgi,mpdc_10_2c_st_avg_hop_latency,l_mpdc_10_2c_st,'LineWidth',linewidth_value);
mpdc_10_3c_st_latency = plot(pgi,mpdc_10_3c_st_avg_hop_latency,l_mpdc_10_3c_st,'LineWidth',linewidth_value);

hold on;
contiki_latency = plot(pgi,contikimac_avg_hop_latency,l_contikimac,'LineWidth',linewidth_value);
cxmac_latency = plot(pgi,cxmac_avg_hop_latency,l_cxmac,'LineWidth',linewidth_value);
nullrdc_latency = plot(pgi,nullrdc_avg_hop_latency,l_nullrdc,'Color',[1 0.5 0],'LineWidth',linewidth_value);

box on;
xlabel('包生成间隔 (s)','fontsize',16); %设置x轴的标题和字体大小
ylabel('端到端延迟 (s)','fontsize',16); %设置y轴的标题和字体大小

AX = legend([ mpdc_10_2c_nst_latency mpdc_10_2c_st_latency mpdc_10_3c_st_latency contiki_latency cxmac_latency nullrdc_latency], ...
     'MPDC (Case 1)','MPDC(Case 2)','MPDC(Case 3)','CCP(ContikiMAC)','CCP(X-MAC)','CCP(Full-Active Radio)','Best');
%  
% AX = legend([ mpdc_10_2c_nst_latency mpdc_10_2c_st_latency mpdc_10_3c_st_latency], ...
%      'MPDC (Case 1)','MPDC(Case 2)','MPDC(Case 3)','Best');
%  
 set(AX,'box','off')

LEG = findobj(AX,'type','text');

set(LEG,'FontSize',11);%设置legend字体大小
set(gca,'FontSize',16);%设置坐标字体大小
grid on;

%% Throughput
figure;
mpdc_10_2c_nst_delivery = plot(pgi,mpdc_10_2c_nst_avg_delivery,l_mpdc_10_2c_nst,'LineWidth',linewidth_value);
hold on;
mpdc_10_2c_st_delivery = plot(pgi,mpdc_10_2c_st_avg_delivery,l_mpdc_10_2c_st,'LineWidth',linewidth_value);
mpdc_10_3c_st_delivery = plot(pgi,mpdc_10_3c_st_avg_delivery,l_mpdc_10_3c_st,'LineWidth',linewidth_value);


hold on;
contiki_delivery = plot(pgi,contikimac_avg_delivery,l_contikimac,'LineWidth',linewidth_value);
cxmac_delivery = plot(pgi,cxmac_avg_delivery,l_cxmac,'LineWidth',linewidth_value);
nullrdc_delivery = plot(pgi,nullrdc_avg_delivery,l_nullrdc,'Color',[1 0.5 0],'LineWidth',linewidth_value);

box on;
xlabel('包生成间隔(s)','fontsize',16); %设置x轴的标题和字体大小
ylabel('吞吐量(packet/s)','fontsize',16); %设置y轴的标题和字体大小
% AX = legend([pdc_data_num_1s cc_data_num_1s],'PDC (PGI=5s)','CC (PGI=5s)',3);%设置legend位置。
% AX = legend([mpdc_10_2c_nst_delivery mpdc_10_2c_st_delivery mpdc_10_3c_st_delivery ], ...
%         'MPDC (Case 1)','MPDC(Case 2)','MPDC(Case 3)','Best');
AX = legend([mpdc_10_2c_nst_delivery mpdc_10_2c_st_delivery mpdc_10_3c_st_delivery contiki_delivery cxmac_delivery nullrdc_delivery], ...
        'MPDC (Case 1)','MPDC(Case 2)','MPDC(Case 3)','CCP(ContikiMAC)','CCP(X-MAC)','CCP(Full-Active Radio)','Best');

 set(AX,'box','off')
     
 LEG = findobj(AX,'type','text');

% axis([0 10 0 1.1]);
set(LEG,'FontSize',11);%设置legend字体大小
set(gca,'FontSize',16);%设置坐标字体大小
grid on;
