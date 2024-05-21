s.boot;
(
SynthDef(\fmSynth,
    {
        arg freq = 440, modIndex = 3, modPartial = 1,
        carPartial = 1, level = 0.05, outBus = 0,
        effectBus, direct = 0.33;
        var modSig, outSig;
        modSig = SinOsc.ar(freq*modPartial,0,freq*modIndex);
        outSig = SinOsc.ar(freq * carPartial + modSig,0,level);
		Out.ar(effectBus,outSig * (1 - direct));
        Out.ar(outBus,(outSig * direct).dup);
		}
).add )

(
           SynthDef(\reverb,
              {
                 arg inBus, outBus;
                 var dry, wet;
                 dry = In.ar (inBus);
                 wet = DelayN.ar(dry,0.048,0.048);
                 wet = Mix.arFill(7,{CombL.ar(wet,0.1,LFNoise1.kr(Rand(0,0.1),0.04,0.05),1 )});
                 4.do({wet = AllpassC.ar(wet,0.05,[Rand(0,0.05),Rand(0,0.05)],15) });
                 Out.ar(outBus, wet);
              }
).add; )

(
var bus = Bus.audio(s);
x= Synth(\fmSynth,[\freq,440,\modIndex,2,\modPartial,2.4,\outBus,0,\level,0.1, \effectBus, bus ]);
y= Synth.after(x,\reverb,[\inBus,bus,\outBus,0]);
)

n = NetAddr("127.0.0.1",57120);

OSCFunc.newMatching({|msg, time, addr, recvPort|x.set(\freq,msg[1]).postln}, 'FMSynth/freq' ,n);
OSCFunc.newMatching({|msg, time, addr, recvPort|x.set(\direct,msg[1]).postln}, '/FMSynth/reverb', n);

	