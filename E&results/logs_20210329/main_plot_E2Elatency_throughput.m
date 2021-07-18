close all 
clc

file_num = [1 2 3 4 5];
pgi = [10 15 20 25 30];

dcpf_10_1c_nst_avg_hop_latency = zeros(5,1);
mpdc_10_2c_nst_avg_hop_latency = zeros(5,1);
mpdc_10_2c_st_avg_hop_latency = zeros(5,1);
mpdc_10_3c_st_avg_hop_latency = zeros(5,1);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
dcpf_10_1c_nst_avg_delivery  = zeros(5,1);
mpdc_10_2c_nst_avg_delivery  = zeros(5,1);
mpdc_10_2c_st_avg_delivery  = zeros(5,1);
mpdc_10_3c_st_avg_delivery  = zeros(5,1);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
dcpf_10_1c_nst_rcvd_pkt_num = zeros(5,1);
mpdc_10_2c_nst_rcvd_pkt_num = zeros(5,1);
mpdc_10_2c_st_rcvd_pkt_num = zeros(5,1);
mpdc_10_3c_st_rcvd_pkt_num = zeros(5,1);


output_data_folder = 'R_data';
data_format = '%s %d %d %s %s %s %d %d %d';
for ii = file_num
    %% dcpf sf =10, with 1 channels
    file_name = [output_data_folder,'/dcpf_10_1C_nst_',int2str(pgi(ii)),'s.txt'];
    disp(file_name)
    [~,sink_id,rcv_time,~,~,~,grade,latency,~]=textread(file_name,data_format);
    dcpf_10_1c_nst_avg_hop_latency(ii) = sum(latency./grade)/length(latency)./128;
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
    dcpf_10_1c_nst_avg_delivery(ii) = total_pkt/total_time;
    
    
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
    
end
l_dcpf_10_1c_nst = 'r--p';l_mpdc_10_2c_nst = 'm->';
l_mpdc_10_2c_st = 'b-o';l_mpdc_10_3c_st = 'c-^';


linewidth_value=2;
%semilogy
dcpf_10_1c_nst_latency = plot(pgi,dcpf_10_1c_nst_avg_hop_latency,l_dcpf_10_1c_nst,'LineWidth',linewidth_value);
hold on;
mpdc_10_2c_nst_latency = plot(pgi,mpdc_10_2c_nst_avg_hop_latency,l_mpdc_10_2c_nst,'LineWidth',linewidth_value);
mpdc_10_2c_st_latency = plot(pgi,mpdc_10_2c_st_avg_hop_latency,l_mpdc_10_2c_st,'LineWidth',linewidth_value);
mpdc_10_3c_st_latency = plot(pgi,mpdc_10_3c_st_avg_hop_latency,l_mpdc_10_3c_st,'LineWidth',linewidth_value);

hold on;

box on;
xlabel('Packet Generation Interval (s)','fontsize',16); %设置x轴的标题和字体大小
ylabel('Average Hop Delivery Latency (s)','fontsize',16); %设置y轴的标题和字体大小

AX = legend([dcpf_10_1c_nst_latency mpdc_10_2c_nst_latency mpdc_10_2c_st_latency mpdc_10_3c_st_latency], ...
     'DCPF','MPDC (Case 1)','MPDC(Case 2)','MPDC(Case 3)','Best');
 
LEG = findobj(AX,'type','text');

set(LEG,'FontSize',11);%设置legend字体大小
set(gca,'FontSize',16);%设置坐标字体大小
grid on;

%% Throughput
figure;

dcpf_10_1c_nst_delivery = plot(pgi,dcpf_10_1c_nst_avg_delivery,l_dcpf_10_1c_nst,'LineWidth',linewidth_value);

hold on;
mpdc_10_2c_nst_delivery = plot(pgi,mpdc_10_2c_nst_avg_delivery,l_mpdc_10_2c_nst,'LineWidth',linewidth_value);
mpdc_10_2c_st_delivery = plot(pgi,mpdc_10_2c_st_avg_delivery,l_mpdc_10_2c_st,'LineWidth',linewidth_value);
mpdc_10_3c_st_delivery = plot(pgi,mpdc_10_3c_st_avg_delivery,l_mpdc_10_3c_st,'LineWidth',linewidth_value);


hold on;

box on;
xlabel('Packet Generation Interval (s)','fontsize',16); %设置x轴的标题和字体大小
ylabel('Throughput (packet/s)','fontsize',16); %设置y轴的标题和字体大小
% AX = legend([pdc_data_num_1s cc_data_num_1s],'PDC (PGI=5s)','CC (PGI=5s)',3);%设置legend位置。
AX = legend([dcpf_10_1c_nst_delivery mpdc_10_2c_nst_delivery mpdc_10_2c_st_delivery mpdc_10_3c_st_delivery], ...
         'DCPF','MPDC (Case 1)','MPDC(Case 2)','MPDC(Case 3)','Best');
LEG = findobj(AX,'type','text');

% axis([0 10 0 1.1]);
set(LEG,'FontSize',11);%设置legend字体大小
set(gca,'FontSize',16);%设置坐标字体大小
grid on;
