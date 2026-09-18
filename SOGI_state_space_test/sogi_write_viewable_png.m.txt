function sogi_write_viewable_png(source, destination, reportRoot)
% Publish only this trial's PNG review image through a hidden native writer.
% MATLAB file protection can make its direct PNG export unreadable externally.
root=char(java.io.File(reportRoot).getCanonicalPath());
src=char(java.io.File(source).getCanonicalPath());
dst=char(java.io.File(destination).getCanonicalPath());
assert(startsWith(src,[root filesep],'IgnoreCase',true) && ...
    startsWith(dst,[root filesep],'IgnoreCase',true),'SOGI:ImageScope','PNG paths must stay inside the report folder.');
[~,~,sx]=fileparts(src); [~,~,dx]=fileparts(dst);
assert(strcmpi(sx,'.png') && strcmpi(dx,'.png'),'SOGI:ImageType','Only PNG exports are accepted.');
fid=fopen(src,'rb'); assert(fid>=0); bytes=fread(fid,Inf,'*uint8'); fclose(fid);
assert(numel(bytes)>=8 && isequal(bytes(1:8).',uint8([137 80 78 71 13 10 26 10])), ...
    'SOGI:PNG','Source must decode as a PNG in MATLAB.');
command=sprintf('$b=[Console]::In.ReadToEnd(); [IO.File]::WriteAllBytes(''%s'',[Convert]::FromBase64String($b))',strrep(dst,'''',''''''));
info=System.Diagnostics.ProcessStartInfo;
info.FileName='C:\Windows\System32\WindowsPowerShell\v1.0\powershell.exe';
info.Arguments=['-NoProfile -NonInteractive -Command "' command '"'];
info.UseShellExecute=false; info.CreateNoWindow=true;
info.WindowStyle=System.Diagnostics.ProcessWindowStyle.Hidden;
info.RedirectStandardInput=true; info.RedirectStandardError=true;
process=System.Diagnostics.Process; process.StartInfo=info; process.Start();
process.StandardInput.Write(matlab.net.base64encode(bytes)); process.StandardInput.Close();
assert(process.WaitForExit(30000),'SOGI:ImageTimeout','PNG writer timed out.');
assert(process.ExitCode==0,'SOGI:ImageWrite','%s',char(process.StandardError.ReadToEnd()));
process.Dispose();
end
