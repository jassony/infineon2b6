function checks = check_sogi_fe_line_separation(reportDir)
%CHECK_SOGI_FE_LINE_SEPARATION Inspect unrelated signal-line intersections.
%   CHECKS = CHECK_SOGI_FE_LINE_SEPARATION(REPORTDIR) reads existing Points
%   in the comparison root, physical-input FE subsystem and validation root.
%   Models must already be loaded. No layout, topology or simulation changes
%   are made. Lines with the same SrcPortHandle are excluded from each other.
arguments
    reportDir {mustBeTextScalar} = fullfile(fileparts(mfilename('fullpath')), ...
        'reports','fe_physical_inputs')
end
reportDir = char(reportDir);
assert(~isempty(reportDir),'SOGI:LineSeparationReport','Report directory must not be empty.');
comparison = 'sogi_discrete_comparison';
validation = 'sogi_validation';
assert(bdIsLoaded(comparison) && bdIsLoaded(validation), ...
    'SOGI:LineSeparationModel','Load both SOGI models before checking line separation.');
found = find_system(comparison,'SearchDepth',1,'Type','block', ...
    'Name','SOGI_DualOutput_Filter_FE');
assert(numel(found)==1,'SOGI:LineSeparationScope', ...
    'Expected one SOGI_DualOutput_Filter_FE subsystem.');
fe = Simulink.ID.getFullName(Simulink.ID.getSID(found{1}));
scopes = {comparison,fe,validation};
rows = cell(0,4);
tolerance = 1e-9;

for scopeIndex = 1:numel(scopes)
    scope = scopes{scopeIndex};
    handles = find_system(scope,'FindAll','on','SearchDepth',1,'Type','line');
    records = struct('Source',{},'Points',{},'Label',{});
    details = cell(0,4);
    invalidCount = 0; diagonalCount = 0;
    for index = 1:numel(handles)
        line = handles(index);
        source = get_param(line,'SrcPortHandle');
        destination = get_param(line,'DstPortHandle');
        if numel(source)~=1 || source<=0 || numel(destination)~=1 || destination<=0
            invalidCount = invalidCount+1;
            details(end+1,:) = {scope,'LineEndpointContract','FAIL', ...
                sprintf('Line handle %.17g is not a complete single-target signal.',line)}; %#ok<AGROW>
            continue
        end
        sourceBlock = get_param(line,'SrcBlockHandle');
        destinationBlock = get_param(line,'DstBlockHandle');
        label = sprintf('%s[%s] -> %s[%s]', ...
            get_param(sourceBlock,'Name'),string(get_param(source,'PortNumber')), ...
            get_param(destinationBlock,'Name'),string(get_param(destination,'PortNumber')));
        points = double(get_param(line,'Points'));
        if size(points,2)~=2 || size(points,1)<2 || ~all(isfinite(points),'all')
            invalidCount = invalidCount+1;
            details(end+1,:) = {scope,'LineGeometry','FAIL', ...
                sprintf('%s: missing or invalid Points.',label)}; %#ok<AGROW>
            continue
        end
        points = points([true;any(abs(diff(points,1,1))>tolerance,2)],:);
        if size(points,1)<2
            invalidCount = invalidCount+1;
            details(end+1,:) = {scope,'LineGeometry','FAIL', ...
                sprintf('%s: no nonzero signal segment.',label)}; %#ok<AGROW>
            continue
        end
        for segment = 1:size(points,1)-1
            if direction(points(segment,:),points(segment+1,:),tolerance)==3
                diagonalCount = diagonalCount+1;
                details(end+1,:) = {scope,'OrthogonalGeometry','FAIL', ...
                    sprintf('%s: diagonal segment [%.9g %.9g] -> [%.9g %.9g].', ...
                    label,points(segment,:),points(segment+1,:))}; %#ok<AGROW>
            end
        end
        records(end+1) = struct('Source',source,'Points',points,'Label',label); %#ok<AGROW>
    end

    pairCount = 0; excludedCount = 0; violatingPairs = 0;
    crossingCount = 0; overlapCount = 0;
    for first = 1:numel(records)-1
        for second = first+1:numel(records)
            if records(first).Source==records(second).Source
                excludedCount = excludedCount+1;
                continue
            end
            pairCount = pairCount+1;
            firstPoints = records(first).Points;
            secondPoints = records(second).Points;
            seen = {};
            for a = 1:size(firstPoints,1)-1
                for b = 1:size(secondPoints,1)-1
                    [kind,geometry] = intersection(firstPoints(a:a+1,:), ...
                        secondPoints(b:b+1,:),tolerance);
                    if isempty(kind); continue; end
                    key = [kind sprintf('|%.12g',geometry)];
                    if any(strcmp(seen,key)); continue; end
                    seen{end+1} = key; %#ok<AGROW>
                    if strcmp(kind,'OrthogonalIntersection')
                        crossingCount = crossingCount+1;
                        location = sprintf('intersection at [%.9g %.9g]',geometry);
                    else
                        overlapCount = overlapCount+1;
                        location = sprintf('overlap [%.9g %.9g] -> [%.9g %.9g]',geometry);
                    end
                    details(end+1,:) = {scope,kind,'FAIL', ...
                        sprintf('%s | %s: %s.',records(first).Label, ...
                        records(second).Label,location)}; %#ok<AGROW>
                end
            end
            if ~isempty(seen); violatingPairs = violatingPairs+1; end
        end
    end
    status = 'PASS';
    if invalidCount>0 || diagonalCount>0 || violatingPairs>0; status = 'FAIL'; end
    summary = sprintf([ ...
        'Lines=%d; checkedPairs=%d; excludedSameSourcePairs=%d; violatingPairs=%d; ', ...
        'perpendicularIntersections=%d; collinearOverlaps=%d; ', ...
        'invalidLines=%d; diagonalSegments=%d.'], ...
        numel(handles),pairCount,excludedCount,violatingPairs,crossingCount, ...
        overlapCount,invalidCount,diagonalCount);
    rows(end+1,:) = {scope,'LineSeparationSummary',status,summary}; %#ok<AGROW>
    rows = [rows;details]; %#ok<AGROW>
    fprintf('%s: %s - %d crossing points, %d overlap segments, %d violating line pairs.\n', ...
        scope,status,crossingCount,overlapCount,violatingPairs);
end
checks = cell2table(rows,'VariableNames',{'Scope','Check','Status','Detail'});
if ~isfolder(reportDir); mkdir(reportDir); end
writetable(checks,fullfile(reportDir,'line_separation_checks.csv'));
end

function value = direction(first,last,tolerance)
delta = abs(last-first);
if all(delta<=tolerance)
    value = 0;
elseif delta(2)<=tolerance
    value = 1; % Horizontal.
elseif delta(1)<=tolerance
    value = 2; % Vertical.
else
    value = 3; % Diagonal: reported separately, not silently certified.
end
end

function [kind,geometry] = intersection(first,second,tolerance)
kind = ''; geometry = [];
firstDirection = direction(first(1,:),first(2,:),tolerance);
secondDirection = direction(second(1,:),second(2,:),tolerance);
if any([firstDirection secondDirection]==0) || any([firstDirection secondDirection]==3)
    return
end
if firstDirection~=secondDirection
    horizontal = first; vertical = second;
    if firstDirection==2
        horizontal = second; vertical = first;
    end
    point = [mean(vertical(:,1)) mean(horizontal(:,2))];
    % Closed intervals deliberately include T-junction endpoints and corners.
    if point(1)>=min(horizontal(:,1))-tolerance && ...
            point(1)<=max(horizontal(:,1))+tolerance && ...
            point(2)>=min(vertical(:,2))-tolerance && ...
            point(2)<=max(vertical(:,2))+tolerance
        kind = 'OrthogonalIntersection'; geometry = point;
    end
    return
end
axis = firstDirection; % Horizontal varies in x; vertical varies in y.
fixedAxis = 3-axis;
if abs(mean(first(:,fixedAxis))-mean(second(:,fixedAxis)))>tolerance
    return
end
low = max(min(first(:,axis)),min(second(:,axis)));
high = min(max(first(:,axis)),max(second(:,axis)));
if high-low<=tolerance
    return % Collinear point contact has no positive-length overlap.
end
ends = zeros(2,2);
ends(:,fixedAxis) = mean([first(:,fixedAxis);second(:,fixedAxis)]);
ends(:,axis) = [low;high];
kind = 'CollinearOverlap'; geometry = reshape(ends.',1,4);
end
