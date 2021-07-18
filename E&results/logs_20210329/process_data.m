% This is to deal with the output data from contiki cooja
% Author: F. Tong
% Date: May 13, 2015

 function [ ] = process_data( separater )
%UNTITLED 此处显示有关此函数的摘要
%  input: separater
%          it could be 'R', 'P', or other characters used to find all
%          the lines which contain this separater.
%%
%file_in = 'data.txt';
SF_array = [10];
protocol_array = char('dcpf','mpdc');
disp(protocol_array)
ch_array = char('1C','2C','3C'); %fa: with free addressing; %nfa: no free addressing
st_array = char('st','nst'); %fa: with free addressing; %nfa: no free addressing
pgi_array = [10 15 20 25 30]; %packet generation interval (unit: second)
input_data_folder = '.';
input_file_name='COOJA.testlog';
output_data_folder = [separater,'_data'];
mkdir(output_data_folder);
for i = pgi_array
    for SF = SF_array
        temp=size(protocol_array);
        protocol_num=temp(1);
        for j = 1:1:protocol_num
            protocol = protocol_array(j,:);
           % disp([protocol,'_'])
            temp=size(ch_array);
            ch_num = temp(1);
            for k = 1:1:ch_num
                ch = ch_array(k,:);
                if strcmp(protocol,'dcpf ') % if fa is 'fa ', the last space should be removed
                    protocol='dcpf';
                end
                if strcmp(protocol,'dcpf') && (strcmp(ch,'2C') || strcmp(ch,'3C'))
                    continue;
                end
               if strcmp(protocol,'mpdc') && strcmp(ch,'1C')
                    continue;
               end
               temp=size(st_array);
               st_num = temp(1);
               for s = 1:1:st_num
                    st = st_array(s,:);
                    if strcmp(st,'st ') % if fa is 'fa ', the last space should be removed
                        st='st';
                    end
                    if strcmp(protocol,'dcpf') && strcmp(st,'st') %'pdc' and 'fa' are not shown together
                        continue;
                    end
                    if strcmp(protocol,'mpdc') &&  strcmp(ch,'3C') &&strcmp(st,'nst') %'pdc' and 'fa' are not shown together
                        continue;
                    end
                   common_file_name = [protocol,'_',int2str(SF),'_',ch,'_',st,'_',int2str(i),'s']
                   file_in=[input_data_folder,'/',common_file_name,'/',input_file_name];
                   disp(file_in)
                    fidin=fopen(file_in,'r+'); % open the file
                    file_out=[output_data_folder,'/',common_file_name,'.txt'];
                    fidout = fopen(file_out,'w+');
                    while ~feof(fidin) % if not the end of the file
                        tline = fgetl(fidin); % read one line
                        if ~isempty(strfind(tline,[' ',separater,' '])) % find the line containing ' R ' or ' P '
                            fprintf(fidout,'%s\n',tline);
                        end
                    end
                    fclose(fidout); % close the ouput file
                    fclose(fidin); % close the input file
               end
            end
        end
    end
end

% file_in_array = char('2.txt','3.txt', '4.txt','5.txt');
% temp = size(file_in_array);
% file_num = temp(1);
% for i=1:1:file_num,
%     file_in = file_in_array(i,:); % get the file name
%     fidin=fopen(file_in,'r+'); % open the file
%     file_out = ['cd_',file_in(1),'.txt']; % set the output file name, e.g., cd_2.txt
%     fidout = fopen(file_out,'w+');
%     while ~feof(fidin) % if not the end of the file
%         tline = fgetl(fidin); % read one line
%         if ~isempty(strfind(tline,' CD ')) % find the line containing ' CD '
%             fprintf(fidout,'%s\n',tline);
%         end
%     end
%     fclose(fidout); % close the ouput file
%     fclose(fidin); % close the input file
% end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%

end

