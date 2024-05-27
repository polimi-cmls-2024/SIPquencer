
// Save this file as PolyphonicSynthManager.sc
PolyphonicSynthManager {
    // Instance variables
	classvar cutoff;
	var notes, program,cutoff,attack,release, outBus; //REMEMBER prog -> symbol

    *new { | prog, output |
        ^super.new.init( prog, output)
    }

    init { | prog, output |
        // do initiation here
        notes = Array.fill(128, { nil }); // Array to hold Synth instances
        program = prog;
		cutoff = 8000;
		attack = 0.5;
		release = 0.2;
		outBus = output;
	}

	setProgram { |programName|
		program = programName.postln;
	}

	print {
		notes.postln;
		program.postln;
	}


    // Method to handle noteOn
    noteOn {
		arg note, velocity;
        // Ensure note is in valid range
        if (note.isInteger and: { note >= 0 and: { note < 128 } }) {


			notes[note] = Synth(program,[\cutoff,cutoff,\attack,attack,\release,release,\freq, note.midicps, \amp, velocity.linexp(1,127,0.01,0.5), \gate, 1, out: outBus]);
        }
    }



    // Method to handle noteOff
    noteOff {
		arg note;
        // Ensure note is in valid range
        if (note.isInteger and: { note >= 0 and: { note < 128 } }) {
			notes[note].set(\gate, 0);
			notes[note] = nil;
        }
    }

	controlChange {
		arg param, value;
		("PARAM:"+ param +"\n").postln;
		("VALUE:"+ value +"\n").postln;
		if(param == 1){
 			// Normalize MIDI control value to the range you want
 			var normalizedValue = value.linexp(0, 127, 20, 20000);

 			cutoff = normalizedValue;
			for(1,notes.size){
				arg i;
				notes[i].set(\cutoff,normalizedValue);
			}

 		};
		if(param == 2){
 			// Normalize MIDI control value to the range you want
 			var normalizedValue = value.linexp(0, 127, 0.01, 0.3);


 			attack = normalizedValue;
			for(1,notes.size){
				arg i;
				notes[i].set(\attack,normalizedValue);
			}

 		};
		if(param == 3){
 			// Normalize MIDI control value to the range you want
 			var normalizedValue = value.linexp(0, 127, 0.01, 3);

 			release = normalizedValue;
			for(1,notes.size){
				arg i;
				notes[i].set(\release,normalizedValue);
			}

 		};
	}


    }

