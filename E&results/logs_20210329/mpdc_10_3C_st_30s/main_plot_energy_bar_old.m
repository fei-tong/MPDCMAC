close all 
file_num = [1 2 3 4 5];
pgi = [10 20 30 40 50];
mpdc_10_2c_array = zeros(5,1);
mpdc_10_3c_array = zeros(5,1);
pdc_10_1c_array = zeros(5,1);
pdc_10_2c_array = zeros(5,1);

data_folder = 'P_data';
data_format='%s %d %s %d %d %d %d'; % original format: '%s %s %d %s %s %s %d %d %d %d'
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
jj = 1;
exc_id_1=1; % excluded id
exc_id_2=2;
for ii = file_num
    %% mpdc sf =10, with 2 channels
    file_name = [data_folder,'/mpdc_10_2C_',int2str(pgi(ii)),'s.txt'];
    [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
    id_index=((ID~=exc_id_1) & (ID~=exc_id_2));% They are sky motes, they will not be conunted
    mpdc_10_2c_array(jj) = sum((control(id_index)+data(id_index)+idle(id_index))./total(id_index))/length(total(id_index));
   
    %% mpdc sf =10, with 3 channels
    file_name = [data_folder,'/mpdc_10_3C_',int2str(pgi(ii)),'s.txt'];
    [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
    id_index=((ID~=exc_id_1) & (ID~=exc_id_2));
    mpdc_10_3c_array(jj) = sum((control(id_index)+data(id_index)+idle(id_index))./total(id_index))/length(total(id_index));

    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %% pdc sf =10, with 1 channel
    file_name = [data_folder,'/pdc_10_1C_',int2str(pgi(ii)),'s.txt'];
    [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
    id_index=((ID~=exc_id_1) & (ID~=exc_id_2));
    pdc_10_1c_array(jj) = sum((control(id_index)+data(id_index)+idle(id_index))./total(id_index))/length(total(id_index));
    
    %% pdc sf =10, with 2 channel
    file_name = [data_folder,'/pdc_10_2C_',int2str(pgi(ii)),'s.txt'];
    [~,ID,~,control,data,idle,total]=textread(file_name,data_format);
    id_index=((ID~=exc_id_1) & (ID~=exc_id_2));
    pdc_10_2c_array(jj) = sum((control(id_index)+data(id_index)+idle(id_index))./total(id_index))/length(total(id_index));
   
    jj=jj+1;
end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
linewidth_value = 2;
l_mpdc_10_2c = 'b-o';l_mpdc_10_3c = 'c-^';
l_pdc_10_1c = 'm->';l_pdc_10_2c = 'r--p';

mpdc_10_2c_dc = plot(pgi,mpdc_10_2c_array*100,l_mpdc_10_2c,'LineWidth',linewidth_value);
hold on;
mpdc_10_3c_dc = plot(pgi,mpdc_10_3c_array*100,l_mpdc_10_3c,'LineWidth',linewidth_value);

pdc_10_1c_dc = plot(pgi,pdc_10_1c_array*100,l_pdc_10_1c,'LineWidth',linewidth_value);
pdc_10_2c_dc = plot(pgi,pdc_10_2c_array*100,l_pdc_10_2c,'LineWidth',linewidth_value);

% 
axis([10 50 1 7]);
box on;
xlabel('Packet Generation Interval (s)','fontsize',16); %设置x轴的标题和字体大小
ylabel('Duty Cycle (%)','fontsize',16);
AX = legend([mpdc_10_2c_dc mpdc_10_3c_dc pdc_10_1c_dc pdc_10_2c_dc],...
    'MPDC (C=2)','MPDC(C=3)','PDC(C=1)','PDC (C=2)','NorthEast');
LEG = findobj(AX,'type','text');

set(LEG,'FontSize',11);%设置legend字体大小
set(gca,'FontSize',16);%设置坐标字体大小
grid on;