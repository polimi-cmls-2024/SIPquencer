s.boot;

(
SynthDef(\fmSynth,
	{
		arg freq = 440, modIndex = 3, modPartial = 1,
		carPartial = 1, level = 0.05, outBus = 0,
		effectBus, direct = 0.33;
		var modSig, outSig;
		modSig = SinOsc.ar(freq*modPartial,
			0,
			freq*modIndex);
		outSig = SinOsc.ar(freq * carPartial + modSig,
			0,
			level);
		Out.ar(effectBus,outSig * (1 - direct));
		Out.ar(outBus,(outSig * direct).dup);
	}
).add
)

(
SynthDef(\reverb,
	{
		arg inBus, outBus;
		var dry, wet;
		dry = In.ar (inBus);
		wet = DelayN.ar(dry,0.048,0.048);
		wet = Mix.arFill(7,{CombL.ar(wet,0.1,LFNoise1.kr(Rand(0,0.1),0.04,0.05),1 )});
		4.do({wet = AllpassC.ar(wet,0.05,
			[Rand(0,0.05),Rand(0,0.05)],15) });
		Out.ar(outBus, wet);
	}
).add;
)

(
var bus = Bus.audio(s);
x = Synth(\fmSynth,[\freq,440,\modIndex,2,\modPartial,2.4,\outBus,0,\level,0.1, \effectBus, bus ]);
y = Synth.after(x,\reverb,[\inBus,bus,\outBus,0]);
)

n = NetAddr("127.0.0.1");



(
SynthDef(\kick, {
    arg out = 0, freq = 50, amp = 0.5, gate = 1, decay = 0.2;
    var sig, env;
    env = EnvGen.kr(Env.perc(0.01, decay), gate, doneAction: 2);
    sig = SinOsc.ar(freq, 0, env * amp);
    Out.ar(out, sig!2);
}).add;
)
Synth(\snare)

OSCFunc.newMatching({|msg, time, addr, 	recvPort| Synth(\snare)},'/FMSynth/freq', 	n);



OSCFunc.newMatching({|msg, time, addr, 	recvPort|x.set(\direct,msg[1]).postln}, 	'/FMSynth/reverb', n);



OSCFunc.trace(bool: true, hideStatusMsg:true)
