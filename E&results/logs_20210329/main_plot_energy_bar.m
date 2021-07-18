close all 
file_num = [1 2 3 4 5];
pgi = [10 15 20 25 30];

dcpf_10_1c_nst_array = zeros(5,1);
mpdc_10_2c_st_array = zeros(5,1);
mpdc_10_2c_nst_array = zeros(5,1);
mpdc_10_3c_st_array = zeros(5,1);

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
    %% pdc sf =10, with 1 channel
    file_name = [data_folder,'/dcpf_10_1C_nst_',int2str(pgi(ii)),'s.txt'];
    [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
    id_index= (ID~=exc_id_1);
    dcpf_10_1c_nst_array(jj) = sum((control(id_index)+data(id_index)+idle(id_index))./total(id_index))/length(total(id_index));
    

    jj=jj+1;
end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
linewidth_value = 2;
l_mpdc_10_2c_st = 'b-o';l_mpdc_10_3c_st = 'c-^';
l_mpdc_10_2c_nst = 'm->';l_dcpf_10_1c_nst = 'r--p';

mpdc_10_2c_st_dc = plot(pgi,mpdc_10_2c_st_array*100,l_mpdc_10_2c_st,'LineWidth',linewidth_value);
hold on;
mpdc_10_2c_nst_dc = plot(pgi,mpdc_10_2c_nst_array*100,l_mpdc_10_2c_nst,'LineWidth',linewidth_value);
mpdc_10_3c_st_dc = plot(pgi,mpdc_10_3c_st_array*100,l_mpdc_10_3c_st,'LineWidth',linewidth_value);

dcpf_10_1c_nst_dc = plot(pgi,dcpf_10_1c_nst_array*100,l_dcpf_10_1c_nst,'LineWidth',linewidth_value);

% 
axis([10 30 1 7]);
box on;
xlabel('Packet Generation Interval (s)','fontsize',16); %设置x轴的标题和字体大小
ylabel('Duty Cycle (%)','fontsize',16);
AX = legend([dcpf_10_1c_nst_dc mpdc_10_2c_nst_dc mpdc_10_2c_st_dc mpdc_10_3c_st_dc],...
    'DCPF','MPDC (Case 1)','MPDC(Case 2)','MPDC(Case 3)','NorthEast');
LEG = findobj(AX,'type','text');

set(LEG,'FontSize',11);%设置legend字体大小
set(gca,'FontSize',16);%设置坐标字体大小
grid on;