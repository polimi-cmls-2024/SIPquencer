s.boot;

// Define the FM Synthesizer
(
SynthDef(\fmSynth,
    {
        arg freq = 440, modIndex = 3, modPartial = 1,
        carPartial = 1, level = 0.05, outBus = 0,
        effectBus, direct = 0.33, dur = 1.0;

        var modSig, outSig, env;

        modSig = SinOsc.ar(freq * modPartial, 0, freq * modIndex);
		env = EnvGen.kr(Env.perc(0.01, dur - 0.02), doneAction: 2);
        outSig = SinOsc.ar(freq * carPartial + modSig, 0, level*env);
        Out.ar(effectBus, outSig * (1 - direct));
        Out.ar(outBus, (outSig * direct).dup);
    }
).add
)

// Define the Reverb Effect
(
SynthDef(\reverb,
    {
        arg inBus, outBus;
        var dry, wet;
        dry = In.ar(inBus);
        wet = DelayN.ar(dry, 0.048, 0.048);
        wet = Mix.arFill(7, { CombL.ar(wet, 0.1, LFNoise1.kr(Rand(0, 0.1), 0.04, 0.05), 1) });
        4.do({ wet = AllpassC.ar(wet, 0.05, [Rand(0, 0.05), Rand(0, 0.05)], 15) });
        Out.ar(outBus, wet);
    }
).add;
)

// Define the Delay Effect
(
SynthDef(\delay,
    {
        arg inBus, outBus, delayTime = 0.5, feedback = 0.5;
        var dry, delayed;
        dry = In.ar(inBus);
        delayed = DelayC.ar(dry, delayTime, delayTime);
        delayed = delayed + (delayed * feedback);
        Out.ar(outBus, dry + delayed);
    }
).add;
)

// Create and connect the Synths
(
var bus1 = Bus.audio(s);
var bus2 = Bus.audio(s);
x = Synth(\fmSynth, [\freq, 440, \modIndex, 2, \modPartial, 2.4, \outBus, 0, \level, 0.1, \effectBus, bus1]);
y = Synth.after(x, \reverb, [\inBus, bus1, \outBus, bus2]);
z = Synth.after(y, \delay, [\inBus, bus2, \outBus, 0]);
)

// Set up OSC messaging to control parameters
n = NetAddr("127.0.0.1");

OSCFunc.newMatching({ |msg, time, addr, recvPort|
    x.set(\freq, msg[1]).postln;
}, '/FMSynth/freq', n);

OSCFunc.newMatching({ |msg, time, addr, recvPort|
    x.set(\direct, msg[1]).postln;
}, '/FMSynth/reverb', n);

OSCFunc.newMatching({ |msg, time, addr, recvPort|
    z.set(\delayTime, msg[1]).postln;
}, '/FMSynth/delayTime', n);

OSCFunc.newMatching({ |msg, time, addr, recvPort|
    z.set(\feedback, msg[1]).postln;
}, '/FMSynth/feedback', n);
