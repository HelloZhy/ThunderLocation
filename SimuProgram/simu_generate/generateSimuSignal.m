function [ eSig, sSigGrp, sFs, eFs ] = generateSimuSignal( t0, tx, ty, tz )
    [yRain, fsRain] = audioread('wind.wav');
    [yThunder, ~] = audioread('thunder1.wav');
    txOffset = fix(fsRain * tx);
    tyOffset = fix(fsRain * ty);
    tzOffset = fix(fsRain * tz);
    
    txSig = yRain;
    txSig(txOffset: txOffset + length(yThunder) - 1) = txSig(txOffset: txOffset + length(yThunder) - 1) + yThunder;
    txSig = (txSig - min(txSig)) / (max(txSig) - min(txSig));
    txSig = uint8(txSig * 256);
    
    tySig = yRain;
    tySig(tyOffset: tyOffset + length(yThunder) - 1) = tySig(tyOffset: tyOffset + length(yThunder) - 1) + yThunder;
    tySig = (tySig - min(tySig)) / (max(tySig) - min(tySig));
    tySig = uint8(tySig * 256);
    
    tzSig = yRain;
    tzSig(tzOffset: tzOffset + length(yThunder) - 1) = tzSig(tzOffset: tzOffset + length(yThunder) - 1) + yThunder;
    tzSig = (tzSig - min(tzSig)) / (max(tzSig) - min(tzSig));
    tzSig = uint8(tzSig * 256);    
    
    sSigGrp = [txSig tySig tzSig];
 
    [yEle, fsEle] = audioread('e_signal_1ms.wav');
    eSig = zeros(fsEle * 20, 1) + wgn(fsEle * 20, 1, -40);
    t0Offset = fix(t0 * fsEle);
    eSig(t0Offset: t0Offset + length(yEle) - 1) = yEle;
    eSig = (eSig - min(eSig)) / (max(eSig) - min(eSig));
    eSig = uint8((eSig + 1) * 256 / 2);
    
    sFs = fsRain;
    eFs = fsEle;
end

