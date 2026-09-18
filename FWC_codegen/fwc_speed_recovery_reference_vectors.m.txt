function vectors = fwc_speed_recovery_reference_vectors()
%FWC_SPEED_RECOVERY_REFERENCE_VECTORS Deterministic reference outputs for C replay.
% Columns: zero-based tick, active, target Q15, filtered Iq Q15 counts, I Q24.
% Matches inputs printed by tests/fwc_speed_recovery_test.c, without a plant.
state = zeros(3,1,'single');
vectors = zeros(160,5);
for tick = 0:159
    direction = 1;
    if tick >= 80
        direction = -1;
    end
    active = (tick >= 4 && tick < 75) || (tick >= 80 && tick < 156);
    request = 16000;
    if mod(tick,40) < 20
        request = 29000;
    end
    error = -100;
    integralAfter = 7999000;
    if mod(tick,16) < 8
        error = 100;
        integralAfter = 8001000;
    end
    kawTs = 256;
    if mod(tick,20) == 0
        kawTs = 0;
    end
    [state,target,integral] = fwc_speed_recovery_reference(state,active,...
        direction*28000,direction*(27000+mod(tick,9)*80),direction*12000,...
        direction*request,direction*error,direction*16000,...
        direction*(12000+mod(tick,5)*600-1200),direction*8000000,...
        direction*integralAfter,kawTs,15,-26215,26214);
    vectors(tick+1,:) = [tick,state(1),target,state(3),integral];
end
end
