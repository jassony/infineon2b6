function sogi_write_review_file(path,bytes)
%SOGI_WRITE_REVIEW_FILE Write review artifacts through a noninteractive host.
% Keeps exported diagram/data files readable by desktop review applications.
% No model, dictionary, configuration or repository commands are executed.
root = fileparts(mfilename('fullpath'));
path = char(java.io.File(path).getCanonicalPath());
allowed = char(java.io.File(fullfile(root,'reports','port_aware_layout')).getCanonicalPath());
assert(startsWith(lower(path),[lower(allowed) filesep]),'SOGI:ReviewPath','Review output must stay in the dedicated report directory.');
encoded = matlab.net.base64encode(uint8(bytes));
command = ['$data=[Console]::In.ReadToEnd();[IO.File]::WriteAllBytes(''' ...
    strrep(path,'''','''''') ''',[Convert]::FromBase64String($data))'];
info = System.Diagnostics.ProcessStartInfo('powershell.exe', ...
    ['-NoProfile -NonInteractive -Command "' command '"']);
info.UseShellExecute=false; info.CreateNoWindow=true;
info.RedirectStandardInput=true; info.RedirectStandardError=true;
process=System.Diagnostics.Process(); process.StartInfo=info;
assert(process.Start(),'SOGI:ReviewWriter','Could not start review writer.');
process.StandardInput.Write(encoded); process.StandardInput.Close();
message=char(process.StandardError.ReadToEnd()); process.WaitForExit();
assert(process.ExitCode==0,'SOGI:ReviewWriter','%s',message);
process.Dispose();
end
