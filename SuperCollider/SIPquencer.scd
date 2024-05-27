s.boot;




/// Utilities ///
OSCdef.trace(true);
OSCdef.trace(false);

s.plotTree;
s.freeAll;
s.reboot;






(
~oscsender=NetAddr("127.0.0.1",12000);

// This next two lines are used if you need to run the code twice to release to fx
// If the code is ran only once it will give two failure in server
r.free();
f.free();




/// GLOBALS ///
~programs = ["SuperSaw", "acid", "fatvelocitybass", "SOSsnare", "kick"];



/// SYNTH DEFINITION ///

(


(
SynthDef(\SuperSaw,{ |freq = 220, gate =0, amp = 0.3, release =1.5, cutoff=10000, q = 10, attack=0.5 , decay=1, sustain=1|

	var vco = Saw.ar([freq, freq*1.01], mul: 1.0, add: 0.0);
	var eg2_params = Env.adsr(attack, decay, sustain, release, 1.0, -4.0);
	var eg2 = EnvGen.kr(eg2_params, gate, doneAction: Done.freeSelf);

	var vcf = BLowPass.ar(vco, cutoff, q, 0.5);


	Out.ar(\out.kr(0), vcf*eg2*amp)
}).add
);


//ACID
(
SynthDef(\acid, {|attack = 0.001, release = 0.04|
	var out = \out.kr(0);
	var sutain = \sustain.kr(1);
	var pan = \pan.kr(0);
	var accelerate = \accelerate.kr(0);
	var freq = \freq.kr(440);
	var amp = \amp.kr(1) * (-12.dbamp);
	var gate = \gate.kr(1);
	var cutoff = \cutoff.kr(440);
	var resonance = \resonance.kr(0.3);
	var env1, env2, son, pitch;
	attack = attack.linlin(0, 1, 0.001, 1);
	pitch = freq.cpsmidi;
	pitch = Lag.kr(pitch, 0.12 * (1-Trig.kr(gate, 0.001)) * gate);
	env1 = EnvGen.ar(Env.new([0, 1.0, 0, 0], [attack, 2.0, release], [0, -4, -4], 2), gate, amp, doneAction:2);
	env2 = EnvGen.ar(Env.adsr(attack, 0.8, 0, release, 70, -4), gate);
	son = LFPulse.ar(pitch.midicps, 0.0, 0.51, 2, -1);

	son = RLPF.ar(son, cutoff + env2.midicps, resonance);

	OffsetOut.ar(\out.kr(0), Pan2.ar(son * env1, pan));
}).add;
);



/// BASS
(
SynthDef(\fatvelocitybass, {|out = 0, freq = 220, amp = 0.5, pan = 0, attack = 0.001, release = 0.5, gate = 0, cutoff = 12000, rq = 0.25, lagTime = 0.01|

	var lfo, osc, filter, env;

	var basefreq = ((freq.lag(lagTime).cpsmidi) + [0, 11.95, 31.03]).midicps;

	osc = Saw.ar(basefreq, [0.5, 0.4, 0.1]) + PinkNoise.ar(Line.kr(1.0, 0, 0.03));

	env = EnvGen.ar(Env.adsr(attack, 1.0, 1.0, release), gate, doneAction: 2);

	filter = BLowPass4.ar(osc, 100 + ((amp.squared) * (freq + cutoff)), rq);

	Out.ar(out, Pan2.ar(Mix(filter) * env * amp * 0.8, pan));
}).add;
);


//Kick
(
SynthDef(\kick,
	{ arg out = 0, attack = 0.001,release = 0.5,cutoff=10000, gate = 0;
	var pitch_contour, drum_osc, drum_lpf, drum_env,freq = 50;
	var  mod_freq = 10, mod_index = 10, sustain = 0.4, amp = 0.8, beater_noise_level = 0.025;
	var beater_source, beater_hpf, beater_lpf, lpf_cutoff_contour, beater_env;
	var kick_mix;
	pitch_contour = Line.kr(freq*4, freq, 0.02);
	drum_osc = PMOsc.ar(	pitch_contour,
				mod_freq,
				mod_index/1.3,
				mul: 1,
				add: 0);
	drum_lpf = LPF.ar(in: drum_osc, freq: 1000, mul: 1, add: 0);
	drum_env = drum_lpf * EnvGen.ar(Env.perc(0.005, sustain), gate, doneAction: 2);
	beater_source = WhiteNoise.ar(beater_noise_level);
	beater_hpf = HPF.ar(in: beater_source, freq: 500, mul: 1, add: 0);
	lpf_cutoff_contour = Line.kr(6000, 500, 0.03);
	beater_lpf = LPF.ar(in: beater_hpf, freq: lpf_cutoff_contour, mul: 1, add: 0);
	beater_env = beater_lpf * EnvGen.ar(Env.perc(attackTime: attack, releaseTime: release, level: 1.0, curve: -4.0), gate, doneAction: 2);

	kick_mix = Mix.new([drum_env, beater_env]) * 2 * amp;
	kick_mix= LPF.ar(kick_mix, cutoff);
	Out.ar(out, [kick_mix, kick_mix])
	}
	).add
);


(
SynthDef(\SOSsnare,
	{arg out = 0, sustain = 0.1, drum_mode_level = 0.25, attack=0.001, gate=0,
	snare_level = 40, snare_tightness = 1000,
	cutoff = 10005, amp = 0.8;

	var snare_noise, snare_brf_1, snare_reson;
	var snare_env;
	var snare_drum_mix, snare_filtered;

	// Choose either noise source below
	// snare_noise = Crackle.ar(2.01, 1);
	snare_noise = LFNoise0.ar(20000, 0.1);
	snare_env = EnvGen.ar(Env.perc(attackTime: attack, releaseTime: 0.1, level: 1.0, curve: -4.0),gate, doneAction: 2);

	snare_drum_mix = snare_noise * 10 * amp * snare_env;

	// Apply low-pass filter
	snare_filtered = LPF.ar(snare_drum_mix, cutoff);

	Out.ar(out, [snare_filtered, snare_filtered])
	}
).add
);




);


//////// FX DEFINITION ////////
(

///REVERB
(
SynthDef.new(\reverb, {
	arg dec=1.5, mix=0.0, lpf1=2000, lpf2=6000, predel=0.025;
	var dry, wet, sig;
	"INSIDE REVERB".postln;
	dry = In.ar (\in.ir(0), 2);
	wet = In.ar (\in.ir(0), 2);
	wet = DelayN.ar (wet, 0.5, predel.clip(0.0001, 0.5));
	wet = 16.collect {
		var temp;
		temp = CombL.ar(
			wet,
			0.1,
			LFNoise1.kr({ExpRand (0.02,0.04)}!2).exprange (0.02,0.099),
			dec
		);
		temp =LPF.ar (temp, lpf1);
	}.sum * 0.25;
	8.do{
		wet = AllpassL.ar(
			wet,
			0.1,
			LFNoise1.kr({ExpRand (0.02,0.04)}!2).exprange (0.02,0.099),
			dec
		);
	};
	wet = LeakDC.ar (wet);
	wet = LPF.ar (wet, lpf2, 0.5);
	sig = dry.blend (wet, mix);
	Out.ar (\out.ir(0), sig);
}).add;
);



(
SynthDef(\fxDelay, {|inBus, outBus, time = 0.5, hp = 50, lp = 10000, pingpong = 0, feedback = 0.6, mix = 0.5|
    var fb = LocalIn.ar(2);
    var lpb = (-2pi*(lp/SampleRate.ir)).exp;
    var hpb = (-2pi*(hp/SampleRate.ir)).exp;
    var input = In.ar(inBus, 2);
    var delayed = DelayL.ar(input + fb, 1.2, time.lag(0.3));
    var lowpassed = OnePole.ar(delayed, lpb);
    var highpassed = lowpassed - OnePole.ar(lowpassed, hpb);
    var normal = 1 - pingpong;
	var wet = highpassed;
    var dry = input;
	var output;
    LocalOut.ar(
        feedback * (
            (normal*highpassed) +
            (pingpong*highpassed.reverse)
        )
    );


    output = LinXFade2.ar(dry, wet, mix * 2 - 1); // -1 for dry, 1 for wet

    Out.ar(outBus, output);
}).add;
);

);



/// BUS ROUTING ///

(
s.newBusAllocators;
~sourceBus = Bus.audio(s, 2);
/*~delayBus = Bus.audio(s, 2);
~mainOutBus = Bus.audio(s, 2);*/
f = Synth(\fxDelay, [\in, ~sourceBus, \out, 0, \mix, 0, \feedback, 0]);
r = Synth(\reverb, [\in, ~sourceBus, \out, 0, \mix, 0]);

);


/// SEQUENCE INSTATIATION ///

(
a = PolyphonicSynthManager.new("SuperSaw", ~sourceBus);
b = PolyphonicSynthManager.new("SuperSaw", ~sourceBus);
c = PolyphonicSynthManager.new("SuperSaw", ~sourceBus);
d = PolyphonicSynthManager.new("SuperSaw", ~sourceBus);

~polysynths = [a, b, c, d];
);



/// OSCDEFs


(
(
OSCdef.new(\NoteOn_listener, {
	arg msg;
	var nn, vel, ch;
	msg.postln;
	nn = msg[1];
    vel = msg[2];
    ch = msg[3];
	("NOTE ON\nnn:"+nn+"\nChannel:"+ch+"\n").postln;
	//(nn+(128*ch)).postln;
/*	"Message: %\nNote Number: %\nVelocity: %\nChannel: %\n".format(msg[0], nn, vel, ch).postln;*/
	~polysynths[ch-1].noteOn(nn, vel);

	~oscsender.sendMsg(\NoteOn, nn, vel, ch);

}, '/NoteOn', nil, 57121);
);


(
OSCdef.new(\NoteOff_listener, {
	arg msg;
	var nn, ch;
	nn = msg[1];
    ch = msg[2];
	("NOTE OFF\nnn:"+nn+"\nChannel:"+ch+"\n").postln;
	//(nn+(128*ch)).postln;
/*	"Message: %\nNote Number: %\nChannel: %\n".format(msg[0], nn, ch).postln;*/

	~polysynths[ch-1].noteOff(nn);

	~oscsender.sendMsg(\NoteOff, nn, ch);


}, '/NoteOff', nil, 57121);
);

(
OSCdef.new(\Controls, {
	arg msg, val, control_num, ch;
	control_num = msg[1];
	val = msg[2];
	ch = msg[3];

	("CONTROL\nNum:"+control_num+"\nChannel:"+ch+"\nValue:"+val+"\n").postln;

	if([1, 2, 3].includes(control_num)){
		~polysynths[ch-1].controlChange(control_num,val);
	};
	if(control_num == 4){
		r.set(\mix, val.linlin(1, 127, 0, 1));
	};
	if(control_num == 5){
		f.set(\mix, val.linlin(1, 127, 0, 1));
	};

	~oscsender.sendMsg(\Controls, control_num, val, ch);

}, '/Control', nil, 57121);

);

(
OSCdef.new(\ProgramChange_listener, {
	arg msg, ch, synthIndex;
	ch = msg[1].postln;
	synthIndex = msg[2];


	~polysynths[ch-1].setProgram(~programs[synthIndex-1]);

}, '/ProgramChange', nil, 57121);
);

)

)












