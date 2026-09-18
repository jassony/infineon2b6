function source = powerfit_source_record(mf4File)
%POWERFIT_SOURCE_RECORD Return immutable provenance information for one log.

mf4File = string(mf4File);
if ~isscalar(mf4File) || strlength(mf4File) == 0 || ~isfile(mf4File)
    error("powerfit:FileNotFound", "MF4 file was not found: %s", mf4File);
end

fileInfo = dir(mf4File);
if numel(fileInfo) ~= 1
    error("powerfit:FileMetadata", "Could not resolve one file record for: %s", mf4File);
end

source = struct();
source.SourceFile = string(fullfile(fileInfo.folder, fileInfo.name));
[~, testId] = fileparts(source.SourceFile);
source.TestId = string(testId);
source.FileBytes = double(fileInfo.bytes);
source.ModifiedTime = datetime(fileInfo.datenum, "ConvertFrom", "datenum");
source.SourceHashSHA256 = localFileSha256(source.SourceFile);
source.SourceId = "sha256:" + source.SourceHashSHA256;
end

function hashText = localFileSha256(fileName)
% Java is available in desktop MATLAB and permits hashing binary MF4 data
% without loading the complete file into memory.
stream = java.io.FileInputStream(char(fileName));
streamCleanup = onCleanup(@() stream.close());
digest = java.security.MessageDigest.getInstance("SHA-256");
buffer = zeros(65536, 1, "int8");

while true
    bytesRead = stream.read(buffer);
    if bytesRead < 0
        break
    end
    digest.update(buffer(1:bytesRead));
end

digestBytes = typecast(digest.digest(), "uint8");
hashText = lower(string(reshape(dec2hex(digestBytes, 2).', 1, [])));
end
