function [trdata,trhdr]=readrf(ftrace, mdversion, nheaderlines)
%This subroutine reads the receiver function trace in text 
%file, which is output of RFeditor3+ or newer with speficied metadata version.
%See the first line of output file for metadata version specifier.
%
%USAGE: [trdata,trhdr]=readrf(ftrace)
%		[trdata,trhdr]=readrf(ftrace, mdversion)
%       [trdata,trhdr]=readrf(ftrace, mdversion, nheaderlines)
%Modified on: 12/31/2015
%Xiaotao Yang on July 22, 2018
%   1. Added version=3; 
%   2. Added examples for all three versions
%
%trace header lines, examples
%{
Choose metadata version code for saved trace from below (1, 2, 3):
  1  - Example below (basic version)  
     %Metadata version 1
     station            : BLO
     start_time         :  4/01/2014  23:56:30.025
     evid               :       1169
     samples            :       6001
     dt                 :   0.025000
     t0                 : -29.987570
     stack_weight       : -9999.0000
     RT_xcorcoe         : -9999.0000
     RFQualityIndex     : -9999.0000
     DeconSuccessIndex  :  0.4620
     niteration         :          3
     nspike             :          2
     epsilon            :   30.69890
     peakamp            :    0.40356
     averamp            :    0.00535
     rawsnr             :    2.50000
  2  - Example below (basic+magnitude)
     %Metadata version 2
     station            : BLO
     start_time         :  4/01/2014  23:56:30.025
     evid               :       1169
     magnitude          : -9999.00
     magtype            : -
     samples            :       6001
     dt                 :   0.025000
     t0                 : -29.987570
     stack_weight       : -9999.0000
     RT_xcorcoe         : -9999.0000
     RFQualityIndex     : -9999.0000
     DeconSuccessIndex  :  0.4620
     niteration         :          3
     nspike             :          2
     epsilon            :   30.69890
     peakamp            :    0.40356
     averamp            :    0.00535
     rawsnr             :    2.50000
  3  - Example below (basic+magnitude+back_azimuth) 
     %Metadata version 3
     station            : BLO
     start_time         :  4/01/2014  23:56:30.025
     evid               :       1169
     magnitude          : -9999.00
     magtype            : -
     back_azimuth       : 172.5
     samples            :       6001
     dt                 :   0.025000
     t0                 : -29.987570
     stack_weight       : -9999.0000
     RT_xcorcoe         : -9999.0000
     RFQualityIndex     : -9999.0000
     DeconSuccessIndex  :  0.4620
     niteration         :          3
     nspike             :          2
     epsilon            :   30.69890
     peakamp            :    0.40356
     averamp            :    0.00535
     rawsnr             :    2.50000
%}

%to make the program compatible with earlier usage, default mdversion is 1.
if(nargin==1) 
    mdversion=1;
    nheaderlines=0;
elseif(nargin==2)
    nheaderlines=1;
end
  
fid=fopen(ftrace);
fprintf('Reading trace from: %s \n',ftrace);

if(mdversion==1)
	trhdr_default=struct('sta','-',...
             'UTCstart','-',...
             'evid',-9999,...
             'nsamp',-9999,...
             'dt',-9999.9,...
             't0',-9999.9,...
             'stackweight',-9999.9,...
             'rt_xcorcoe',-9999.9,...
             'rfqi',-9999.9,...
             'dsi',-9999.9,...
             'niteration',-9999,...
             'nspike',-9999,...
             'epsilon',-9999.9,...
             'peakamp',-9999.9,...
             'averamp',-9999.9,...
             'rawsnr',-9999.9);    %trace header
	
	trhdr=trhdr_default;
    for k=1:nheaderlines
        temp=fgetl(fid);
    end
    temp=fgetl(fid);
    
    k=strfind(temp,':');
    trhdr.sta=sscanf(temp((k+1):length(temp)),'%s');
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.UTCstart=sscanf(temp((k+1):length(temp)),'%s');
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.evid=sscanf(temp((k+1):length(temp)),'%d');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.nsamp=sscanf(temp((k+1):length(temp)),'%d');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.dt=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.t0=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.stackweight=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.rt_xcorcoe=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.rfqi=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.dsi=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.niteration=sscanf(temp((k+1):length(temp)),'%d');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.nspike=sscanf(temp((k+1):length(temp)),'%d');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.epsilon=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.peakamp=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.averamp=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.rawsnr=sscanf(temp((k+1):length(temp)),'%g');

elseif(mdversion==2)
	trhdr_default=struct('sta','-',...
             'UTCstart','-',...
             'evid',-9999,...
             'magnitude',-9999.9,...
             'magtype','-',...
             'nsamp',-9999,...
             'dt',-9999.9,...
             't0',-9999.9,...
             'stackweight',-9999.9,...
             'rt_xcorcoe',-9999.9,...
             'rfqi',-9999.9,...
             'dsi',-9999.9,...
             'niteration',-9999,...
             'nspike',-9999,...
             'epsilon',-9999.9,...
             'peakamp',-9999.9,...
             'averamp',-9999.9,...
             'rawsnr',-9999.9);    %trace header  

	trhdr=trhdr_default;
    for k=1:nheaderlines
        temp=fgetl(fid);
    end
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.sta=sscanf(temp((k+1):length(temp)),'%s');
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.UTCstart=sscanf(temp((k+1):length(temp)),'%s');
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.evid=sscanf(temp((k+1):length(temp)),'%d');
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.magnitude=sscanf(temp((k+1):length(temp)),'%g');
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.magtype=sscanf(temp((k+1):length(temp)),'%s');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.nsamp=sscanf(temp((k+1):length(temp)),'%d');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.dt=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.t0=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.stackweight=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.rt_xcorcoe=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.rfqi=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.dsi=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.niteration=sscanf(temp((k+1):length(temp)),'%d');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.nspike=sscanf(temp((k+1):length(temp)),'%d');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.epsilon=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.peakamp=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.averamp=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.rawsnr=sscanf(temp((k+1):length(temp)),'%g');
elseif(mdversion==3)
	trhdr_default=struct('sta','-',...
             'UTCstart','-',...
             'evid',-9999,...
             'magnitude',-9999.9,...
             'magtype','-',...
             'back_azimuth',-9999.9,...
             'nsamp',-9999,...
             'dt',-9999.9,...
             't0',-9999.9,...
             'stackweight',-9999.9,...
             'rt_xcorcoe',-9999.9,...
             'rfqi',-9999.9,...
             'dsi',-9999.9,...
             'niteration',-9999,...
             'nspike',-9999,...
             'epsilon',-9999.9,...
             'peakamp',-9999.9,...
             'averamp',-9999.9,...
             'rawsnr',-9999.9);    %trace header  

	trhdr=trhdr_default;
    for k=1:nheaderlines
        temp=fgetl(fid);
    end
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.sta=sscanf(temp((k+1):length(temp)),'%s');
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.UTCstart=sscanf(temp((k+1):length(temp)),'%s');
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.evid=sscanf(temp((k+1):length(temp)),'%d');
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.magnitude=sscanf(temp((k+1):length(temp)),'%g');
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.magtype=sscanf(temp((k+1):length(temp)),'%s');
    k=strfind(temp,':');
    trhdr.back_azimuth=sscanf(temp((k+1):length(temp)),'%g');
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.nsamp=sscanf(temp((k+1):length(temp)),'%d');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.dt=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.t0=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.stackweight=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.rt_xcorcoe=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.rfqi=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.dsi=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.niteration=sscanf(temp((k+1):length(temp)),'%d');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.nspike=sscanf(temp((k+1):length(temp)),'%d');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.epsilon=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.peakamp=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.averamp=sscanf(temp((k+1):length(temp)),'%g');
    
    temp=fgetl(fid);
    k=strfind(temp,':');
    trhdr.rawsnr=sscanf(temp((k+1):length(temp)),'%g');

else
	error('ERROR: wrong metadata version specifier, only use 1 or 2. Default is 1.');   
end

trdata=fscanf(fid,'%g',[1,inf]);

fclose(fid);

return;
end