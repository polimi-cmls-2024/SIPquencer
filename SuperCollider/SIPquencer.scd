s.boot;


//MIDI SETUP
/*(
MIDIClient.init;
MIDIIn.connectAll;
//MIDIIn.connect(0,0) to connect only the first port
MIDIClient.sources;
)*/

OSCFunc.trace(true);  //only if needed
OSCFunc.trace(false);

NetAddr.langPort;

~touchosc = NetAddr.new("127.0.0.1");

//address: a NetAddr(127.0.0.1, 57669)
//recvPort: 57120

(
OSCdef.new(\NoteOn_listener, {
	arg msg;
	msg.postln;
	//code that does the thing we want to happen goes here
}, '/NoteOn', nil, 57120);
)

(
OSCdef.new(\NoteOff_listener, {
	arg msg;
	msg.postln;
	//code that does the thing we want to happen goes here
}, '/NoteOff', nil, 57120);
)


(

~notes = Array.newClear(128);
~cutoff = 15000;
~q_factor = 1;
~attack = 1;
~decay = 0.1;
~sustain = 0.7;
~release = 1.5;
~prog = "SuperSaw";


(
OSCdef.new(\NoteOn_listener, {
	arg msg, nn, vel, ch;
	// msg[1] = note number,
	// msg[2] = velocity,
	// msg[3] = channel,
	nn = msg[1];
    vel = msg[2];
    ch = msg[3];
/*	"Message: %\nNote Number: %\nVelocity: %\nChannel: %\n".format(msg[0], nn, vel, ch).postln;*/

	~notes[nn] = Synth.new(~prog,
		[
			\freq, nn.midicps,
			/*			\amp, vel.linexp (1,127,0.01,0.4),*/
			\gate, 1,
			\cutoff, ~cutoff,
			\q, ~q_factor,
			\attack, ~attack,
			\decay, ~decay,
			\susatin, ~sustain,
			\release, ~release,

		]
	);

}, '/NoteOn', nil, 57120);
);



(
OSCdef.new(\NoteOff_listener, {
	arg msg, nn, ch;
	// msg[1] = note number,
	// msg[2] = velocity,
	// msg[3] = channel,
	nn = msg[1];
    ch = msg[2];
/*	"Message: %\nNote Number: %\nChannel: %\n".format(msg[0], nn, ch).postln;*/

	~notes[nn].set(\gate, 0);
	~notes[nn] = nil;

	//code that does the thing we want to happen goes here
}, '/NoteOff', nil, 57120);
);

)





(
OSCdef.new(\Controls, {
	arg msg, val, control_num;
	control_num = msg[1];
	val = msg[2];

		if(control_num == 1){
 			// Normalize MIDI control value to the range you want for eg1_freq (e.g., 0-20000)
 			var normalizedValue = val.linexp(0, 127, 20, 20000);

 			// Update the Synth parameters
 			// Note: We use 'set' instead of 'at' to update the parameter
 			~cutoff = normalizedValue;
 		};
		if(control_num == 2){
 			// Normalize MIDI control value to the range you want for eg1_freq (e.g., 0-20000)
 			var normalizedValue_2 = val.linlin(0, 127, 10, 0.5);

 			// Update the Synth parameters
 			// Note: We use 'set' instead of 'at' to update the parameter
 			~q_factor = normalizedValue_2;
 		};
		if(control_num == 5){
 			// Normalize MIDI control value to the range you want for eg1_freq (e.g., 0-20000)
 			var normalizedValue_5 = val.linlin(0, 127, 0.01, 10);

 			// Update the Synth parameters
 			// Note: We use 'set' instead of 'at' to update the parameter
 			~attack = normalizedValue_5;
 		};
			if(control_num == 6){
 			// Normalize MIDI control value to the range you want for eg1_freq (e.g., 0-20000)
 			var normalizedValue_6 = val.linlin(0, 127, 0.1, 1);

 			// Update the Synth parameters
 			// Note: We use 'set' instead of 'at' to update the parameter
 			~decay = normalizedValue_6;
 		};
			if(control_num == 7){
 			// Normalize MIDI control value to the range you want for eg1_freq (e.g., 0-20000)
 			var normalizedValue_7 = val.linlin(0, 127, 0.1, 1);

 			// Update the Synth parameters
 			// Note: We use 'set' instead of 'at' to update the parameter
 			~sustain = normalizedValue_7;
 		};
			if(control_num == 8){
 			// Normalize MIDI control value to the range you want for eg1_freq (e.g., 0-20000)
 			var normalizedValue_8 = val.linlin(0, 127, 0.1, 3);

 			// Update the Synth parameters
 			// Note: We use 'set' instead of 'at' to update the parameter
 			~release = normalizedValue_8;
 		};


}, '/Control', nil, 57120);
)




(
OSCdef.new(\ProgramChange_listener, {
	arg msg, prog;
	msg.postln;
	//code that does the thing we want to happen goes here
	prog = msg[1];
	prog.postln;

	if(prog==0)
	{
		~prog = "SuperSaw";
	};
	if(prog==1)
	{
		~prog = "SuperSawAlt";
	};

}, '/ProgramChange', nil, 57120);
)





// SYNTHS DEFINITIONS



(
SynthDef(\SuperSaw,{ |out, freq = 220, gate =1, amp = 0.7, release =1.5, cutoff=10000, q = 4, attack=0.5 , decay=0.1, sustain=0.7|

	var vco = Saw.ar([freq, freq*1.01], mul: 1.0, add: 0.0);
	var eg2_params = Env.adsr(attack, decay, sustain, release, 1.0, -4.0);
	var eg2 = EnvGen.kr(eg2_params, gate, doneAction: Done.freeSelf);

	var vcf = BLowPass.ar(vco, cutoff, q, 0.5);


	Out.ar(out!2, vcf*eg2*amp)
}).add
)


(
SynthDef(\SuperSawDet,{ |out, freq = 220, gate =1, amp = 0.7, release =1.5, cutoff=10000, q = 4, attack=0.5 , decay=0.1, sustain=0.7|

	var vco = Saw.ar([freq, freq*1.03], mul: 1.0, add: 0.0);
	var eg2_params = Env.adsr(attack, decay, sustain, release, 1.0, -4.0);
	var eg2 = EnvGen.kr(eg2_params, gate, doneAction: Done.freeSelf);

	var vcf = BLowPass.ar(vco, cutoff, q, 0.5);


	Out.ar(out!2, vcf*eg2*amp)
}).add
)

(
SynthDef(\SuperSawAlt, { |out=0, freq=220, gate=1, amp=0.7, release=1.5, cutoff=10000, q=4, attack=0.5, decay=0.1, sustain=0.7|
	var pulse = Pulse.ar(freq, 0.5); // Pulse wave
	var saw = Saw.ar(freq * [1, 1.01], mul: 0.5); // Saw wave with slight detuning
	var vco = pulse + saw; // Combine the two waveforms
	var env = EnvGen.kr(Env.adsr(attack, decay, sustain, release), gate, doneAction: Done.freeSelf); // Envelope generator
	var vcf = BLowPass.ar(vco, cutoff, q, 0.5); // Low-pass filter
	Out.ar(out, vcf * env * amp); // Output
}).add;
)







