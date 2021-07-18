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
protocol_array = char('xmac');
disp(protocol_array)

pgi_array = [10 15 20 25 30]; %packet generation interval (unit: second)
input_data_folder = '.';
input_file_name='COOJA.testlog';
output_data_folder = [separater,'_data'];
mkdir(output_data_folder);
for i = pgi_array
        temp=size(protocol_array);
        protocol_num=temp(1);
        for j = 1:1:protocol_num
            protocol = protocol_array(j,:);
             if strcmp(protocol,'nullrdc   ') % if fa is 'fa ', the last space should be removed
                protocol='nullrdc';
            end
           common_file_name = [protocol,'_',int2str(i),'s']
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

%%%%%%%%%%%%%%%%%%%%%%%%%%%%

end

