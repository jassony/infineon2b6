function [nextState, idFw, idRef, active, valid, saturated, idAtLo, status, ...
    reqFlt, actFlt, weakAct, recoveryAct] = fwc_reference_step( ...
    enable, reset, eligible, vdcValid, vutilReq, vutilAct, idBase, ...
    state, params, floorSatTicks, unsatTicks)
%FWC_REFERENCE_STEP Explicit single-precision 500 us FWC reference.
% Params: target, Kp, Ki, IdLo_PU, down/up_PUps, SatEps,
%         LpfTau_ms, Hyst_PU, Enter_ms, Exit_ms.
% State: xi, Id magnitude, request/actual LPF, LPF primed, weak latch,
% enter/exit counts, recovery latch/count, previous target/hysteresis,
% previous enter/exit ticks, previous SatEps, calibration primed.
% Fast streak inputs count consecutive 50 us samples, with floorSatTicks
% beginning only after the final Id reference reaches the floor.
%#codegen
Ts = single(0.0005);
q15 = single(32768);
nextState = zeros(16, 1, 'single');
idFw = single(0); idRef = single(idBase);
reqFlt = single(0); actFlt = single(0);
active = uint8(0); valid = uint8(0); saturated = uint8(0);
idAtLo = uint8(0); status = uint8(0);
weakAct = uint8(0); recoveryAct = uint8(0);
if enable == 0 || reset ~= 0
    return
end
if eligible == 0
    status = uint8(4);
    return
end
if vdcValid == 0 || ~finiteInput(vutilReq) || ~finiteInput(vutilAct) ...
        || vutilReq < 0 || vutilAct < 0
    status = uint8(5);
    return
end
target = bounded(params(1), single(0), single(1), single(0.95));
kp = bounded(params(2), single(0), single(100), single(0));
ki = bounded(params(3), single(0), single(10000), single(0));
idLo = bounded(params(4), single(-13107)/q15, single(0), single(0));
down = bounded(params(5), single(0), single(100), single(0));
up = bounded(params(6), single(0), single(100), single(0));
eps = bounded(params(7), single(0), single(1), single(0));
tau = bounded(params(8), single(0), single(100), single(5));
hystMax = min(single(0.05), min(target, single(1)-target));
hyst = bounded(params(9), single(0), hystMax, single(0.02));
enterTicks = ceil(bounded(params(10), single(0), single(100), single(2))*single(2));
exitTicks = ceil(bounded(params(11), single(0), single(1000), single(20))*single(2));
hi = target+hyst; lo = target-hyst;
s = single(state);
if s(16) == 0 || s(11) ~= target || s(12) ~= hyst || s(13) ~= enterTicks
    s(7) = single(0);
end
if s(16) == 0 || s(11) ~= target || s(12) ~= hyst || s(14) ~= exitTicks
    s(8) = single(0);
end
if s(16) == 0 || s(14) ~= exitTicks || s(15) ~= eps
    s(10) = single(0);
end
s(11:16) = [target; hyst; enterTicks; exitTicks; eps; single(1)];
if s(5) == 0 || tau == 0
    s(3) = single(vutilReq); s(4) = single(vutilAct); s(5) = single(1);
else
    alpha = single(0.5)/(tau+single(0.5));
    s(3) = s(3)+alpha*(single(vutilReq)-s(3));
    s(4) = s(4)+alpha*(single(vutilAct)-s(4));
end
reqFlt = s(3); actFlt = s(4);
if s(6) == 0
    s(7) = countCondition(reqFlt > hi, s(7), enterTicks);
    if reqFlt > hi && s(7) >= enterTicks
        s(6) = single(1); s(7) = single(0);
    end
end
err = single(0);
if s(6) ~= 0
    if reqFlt > hi
        err = min(reqFlt-hi, single(1));
    elseif reqFlt < lo
        err = max(reqFlt-lo, single(-1));
    end
end
maxId = -idLo;
u = single(0);
if s(6) ~= 0
    uRaw = kp*err+s(1);
    u = min(max(uRaw, single(0)), maxId);
    if ~((uRaw >= maxId && err > 0) || (uRaw <= 0 && err < 0))
        s(1) = min(max(s(1)+ki*Ts*err, -maxId), maxId);
        u = min(max(kp*err+s(1), single(0)), maxId);
    end
end
if u > s(2)
    s(2) = s(2)+min(u-s(2), down*Ts);
else
    s(2) = s(2)-min(s(2)-u, up*Ts);
end
s(2) = min(max(s(2), single(0)), maxId);
idFw = -s(2);
idRef = max(min(min(single(idBase),single(0)),idFw),idLo);
% Use rounded Q15 for the same floor/zero decisions as the adapter.
idAtLo = uint8(fix(idRef*q15-single(0.5)) <= fix(idLo*q15));
saturated = uint8((single(vutilReq)-single(vutilAct)) > eps);
if s(9) == 0
    s(10) = single(0);
    if saturated ~= 0 && idAtLo ~= 0 && floorSatTicks >= 40
        s(9) = single(1);
    end
else
    clearSat = saturated == 0 && unsatTicks >= 40 ...
        && (reqFlt-actFlt) <= single(0.5)*eps;
    s(10) = countCondition(clearSat, s(10), exitTicks);
    if clearSat && s(10) >= exitTicks
        s(9) = single(0); s(10) = single(0);
    end
end
exitWeak = s(6) ~= 0 && reqFlt < lo ...
    && fix(s(2)*q15+single(0.5)) <= 1 && s(9) == 0;
s(8) = countCondition(exitWeak, s(8), exitTicks);
if exitWeak && s(8) >= exitTicks
    s(1) = single(0); s(2) = single(0); s(6) = single(0); s(8) = single(0);
    idFw = single(0);
    idRef = min(max(single(idBase),idLo),single(0));
end
nextState = s;
weakAct = uint8(s(6)); recoveryAct = uint8(s(9));
active = uint8(1); valid = uint8(1); status = uint8(1);
if recoveryAct ~= 0
    status = uint8(3);
elseif saturated ~= 0
    status = uint8(2);
end
end

function y = bounded(x, lo, hi, fallback)
if ~finiteInput(x)
    x = fallback;
end
y = min(max(single(x),lo),hi);
end

function ok = finiteInput(x)
ok = isfinite(x) && abs(x) < single(1e6);
end

function n = countCondition(condition, previous, ticks)
n = single(0);
if condition
    n = min(previous+single(1),ticks);
end
end
