close all 
clc

file_num = [1 2 3 4 5];
pgi = [10 20 30 40 50];

mpdc_10_2c_avg_hop_latency = zeros(5,1);
mpdc_10_3c_avg_hop_latency = zeros(5,1);
pdc_10_1c_avg_hop_latency = zeros(5,1);
pdc_10_2c_avg_hop_latency = zeros(5,1);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
mpdc_10_2c_avg_delivery  = zeros(5,1);
mpdc_10_3c_avg_delivery  = zeros(5,1);
pdc_10_1c_avg_delivery  = zeros(5,1);
pdc_10_2c_avg_delivery  = zeros(5,1);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
mpdc_10_2c_rcvd_pkt_num = zeros(5,1);
mpdc_10_3c_rcvd_pkt_num = zeros(5,1);
pdc_10_1c_rcvd_pkt_num = zeros(5,1);
pdc_10_2c_rcvd_pkt_num = zeros(5,1);

output_data_folder = 'R_data';
data_format = '%s %d %d %s %s %s %d %d %d';
for ii = file_num
    %% mpdc sf =10, with 2 channels
    file_name = [output_data_folder,'/mpdc_10_2C_',int2str(pgi(ii)),'s.txt'];
    disp(file_name)
    [~,sink_id,rcv_time,~,~,~,grade,latency,~]=textread(file_name,data_format);
    mpdc_10_2c_avg_hop_latency(ii) = sum(latency./grade)/length(latency)./128;
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
    mpdc_10_2c_avg_delivery(ii) = total_pkt/total_time;
    
    %% mpdc sf =10, with 3 channels
    file_name = [output_data_folder,'/mpdc_10_3C_',int2str(pgi(ii)),'s.txt'];
    [~,sink_id,rcv_time,~,~,~,grade,latency,~]=textread(file_name,data_format);

    mpdc_10_3c_avg_hop_latency(ii) = sum(latency./grade)/length(latency)./128;
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
    mpdc_10_3c_avg_delivery(ii) = total_pkt/total_time;
    
    %% pdc sf =10, with 1 channels
    file_name = [output_data_folder,'/pdc_10_1C_',int2str(pgi(ii)),'s.txt'];
    [~,sink_id,rcv_time,~,~,~,grade,latency,~]=textread(file_name,data_format);
    pdc_10_1c_avg_hop_latency(ii) = sum(latency./grade)/length(latency)./128;
 %   pdc_18_nfa_rcvd_pkt_num(ii) = length(latency);
    
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
    pdc_10_1c_avg_delivery(ii) = total_pkt/total_time;
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %% pdc sf =10, with 2 channels
    file_name = [output_data_folder,'/pdc_10_2C_',int2str(pgi(ii)),'s.txt'];
    [~,sink_id,rcv_time,~,~,~,grade,latency,~]=textread(file_name,data_format);
    pdc_10_2c_avg_hop_latency(ii) = sum(latency./grade)/length(latency)./128;
 %   adc_10_fa_rcvd_pkt_num(ii) = length(latency);
    
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
    pdc_10_2c_avg_delivery(ii) = total_pkt/total_time;
    
end
l_mpdc_10_2c = 'b-o';l_mpdc_10_3c = 'c-^';
l_pdc_10_1c = 'm->';l_pdc_10_2c = 'r--p';

linewidth_value=2;
%semilogy
mpdc_10_2c_latency = plot(pgi,mpdc_10_2c_avg_hop_latency,l_mpdc_10_2c,'LineWidth',linewidth_value);
hold on;
mpdc_10_3c_latency = plot(pgi,mpdc_10_3c_avg_hop_latency,l_mpdc_10_3c,'LineWidth',linewidth_value);
pdc_10_1c_latency = plot(pgi,pdc_10_1c_avg_hop_latency,l_pdc_10_1c,'LineWidth',linewidth_value);
pdc_10_2c_latency = plot(pgi,pdc_10_2c_avg_hop_latency,l_pdc_10_2c,'LineWidth',linewidth_value);

hold on;

box on;
xlabel('Packet Generation Interval (s)','fontsize',16); %设置x轴的标题和字体大小
ylabel('Average Hop Delivery Latency (s)','fontsize',16); %设置y轴的标题和字体大小

AX = legend([mpdc_10_2c_latency mpdc_10_3c_latency pdc_10_1c_latency pdc_10_2c_latency], ...
     'MPDC (C=2)','MPDC(C=3)','PDC(C=1)','PDC (C=2)','Best');
LEG = findobj(AX,'type','text');

set(LEG,'FontSize',11);%设置legend字体大小
set(gca,'FontSize',16);%设置坐标字体大小
grid on;

%% Throughput
figure;
mpdc_10_2c_delivery = plot(pgi,mpdc_10_2c_avg_delivery,l_mpdc_10_2c,'LineWidth',linewidth_value);
hold on;
mpdc_10_3c_delivery = plot(pgi,mpdc_10_3c_avg_delivery,l_mpdc_10_3c,'LineWidth',linewidth_value);
pdc_10_1c_delivery = plot(pgi,pdc_10_1c_avg_delivery,l_pdc_10_1c,'LineWidth',linewidth_value);
pdc_10_2c_delivery = plot(pgi,pdc_10_2c_avg_delivery,l_pdc_10_2c,'LineWidth',linewidth_value);

hold on;

box on;
xlabel('Packet Generation Interval (s)','fontsize',16); %设置x轴的标题和字体大小
ylabel('Throughput (packet/s)','fontsize',16); %设置y轴的标题和字体大小
% AX = legend([pdc_data_num_1s cc_data_num_1s],'PDC (PGI=5s)','CC (PGI=5s)',3);%设置legend位置。
AX = legend([mpdc_10_2c_delivery mpdc_10_3c_delivery pdc_10_1c_delivery pdc_10_2c_delivery], ...
     'MPDC (C=2)','MPDC(C=3)','PDC(C=1)','PDC (C=2)','Best');
LEG = findobj(AX,'type','text');

% axis([0 10 0 1.1]);
set(LEG,'FontSize',11);%设置legend字体大小
set(gca,'FontSize',16);%设置坐标字体大小
grid on;
