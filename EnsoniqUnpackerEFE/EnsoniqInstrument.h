#ifndef _ENSONIQINSTRUMENT_H_
#define _ENSONIQINSTRUMENT_H_

//---------------------------------------------------------------------------
typedef struct _EPSNAME
{
	WORD Data[12];
} EPSNAME;

typedef struct _EPSOFFSET
{
	BYTE Data[4];
} EPSOFFSET;

typedef struct _EPSLONG
{
	BYTE Data[4];
} EPSLONG;

typedef struct _FXHEADER
{
	EPSLONG HeaderSize;
	WORD SavedFrom;
	WORD Unknown1;
	WORD Unknown2;
	WORD Name[12];
	BYTE FxSize[4];
	WORD Microcode;
	WORD ESPTable;
	WORD InitCode;
	WORD UpdateTask;
	WORD Var[4];
	WORD ParListStart;
	WORD ParListLast;
	WORD ParListCurrent;
	WORD FXName[3][13];
	WORD CurVar;
	BYTE OutGPRL;
	BYTE OutGPRR;
	WORD NumVoices; /* 0 = 20, 1 = 13, 2 = 7 */
	WORD KeyDownRoutine;
} FXHEADER;

typedef struct _INSTRUMENTHEADER
{
	EPSLONG HeaderSize;
	WORD  SavedFrom;
	BYTE  Invariant[4];
	EPSNAME InstName;             //
	WORD  MIDIChannel,            // 0...15, Anzeige: 1...16
	MIDIProgram,            // 1...128, Anzeige: 1...128
	MIDIPressure,           // 0...2: OFF, KEY, CHAN
	BlocksSize,             //
	KeyDest,                // 0...4: BOTH, LOCAL, MIDI, OFF, EXT
	PatchSel[4],            // Layers für Patches: 00, 0X, X0, XX, Bit 0 = Layer 1
	KeyDown,                // KeyDown-Layers, Bit 0 = Layer 1
	KeyUp,                  // KeyUp-Layers, Bit 0 = Layer 1
	PatchSelMode,           // 0...6, LIVE, 00, 0X, X0, XX, HELD
	PedalDown,              // PedalDown-Layers ??? *undocumented*
	ID,                     // 0 für EPS Classic, FF für EPS 16+
	LowKey,                 // 0...127: C-1 bis G9
	HighKey;                // 0...127: C-1 bis G9
	short Transpose;
	EPSOFFSET  PitchTableOfs[8],
	LayerOfs[8],
	WaveOfs[128],
	EffectOfs;
	BYTE  undefined[8];
} INSTRUMENTHEADER;

typedef struct _LAYERHEADER
{
	EPSLONG HeaderSize;
	WORD  SavedFrom,
	LayerNum,               // ?
	LayerNumAgain;          // ?
	EPSNAME LayerName;
	char  GlideMode,        // 0...4: OFF, LEGATO, PEDAL, TRIGGER, MINIMODE
	DelayVelAmt,            // -127...127, Anzeige -99...99
	GlideTime,              // 0...99
	RestrikeDelay;          // 0...99
	WORD  LegatoLayer,      // 0...7, Anzeige 1...8
	VelLo,                  // 0..127
	VelHi,                  // 0..127
	PitchTable,             // 0..9, STANDARD, NO PITCH, PT1...8
	LayerDelay;             // 0...5000 ms (Motorola order)
	WORD  LayerMap[88];     // 0..127
} LAYERHEADER;

typedef struct _PITCHTABLEHEADER
{
	EPSLONG HeaderSize;
	WORD  SavedFrom,
	Unknown1,
	Unknown2;
	EPSNAME PitchName;
	WORD  Semitones[88],
	Fractions[88];
} PITCHTABLEHEADER;

typedef struct _ENVELOPEPOINT
{
	WORD Soft, Hard, Time;
} ENVELOPEPOINT;

typedef struct _WAVEPOSITION
{
	WORD Data[3];
} WAVEPOSITION;

typedef struct _ENV
{
	WORD EnvType;				// for defaults
	ENVELOPEPOINT Point[5];
	WORD SoftVelCurve,			// on or off
	ReleaseLevel,
	ReleaseTime,
	AttackTimeVel,				// time 0 mod by velocity
	KeyScale,					// scale by keyboard position
	Mode;						// 0 = normal, 1 = cycle, 2 = repeat
} ENV;

typedef struct _WAVESAMPLEHEADER
{
	EPSLONG	HeaderSize;
	WORD	SavedFrom,
			Zero1,
			Zero2;
	EPSNAME WSName;
	WORD	CopyWave,				// if copy, number of ws
		 	CopyLayer;				// if copy, layer of ws
	ENV		Envelope[3];			// envelope 0 (pitch)
									// envelope 1 (filter)
									// envelope 2 (amp)
	BYTE	RootKey,				// 0...127: C-1 bis G9
			AModCurve;
	WORD	PitchEnvAmt;
	char	LFOAmt,
			LA0;
	WORD	RandomAmt;
	char	PitchBend,
			MixerAModCurve;
	WORD	ModSource;
	char	FineTune,
			FT0;
	WORD	ModAmt;
	char	FilterMode,
			MixerBModSource;
	WORD	FC1Cutoff,
			FC2Cutoff,
			FC1KeyAmt,
			FC2KeyAmt,
			FC1EnvAmt,
			FC2EnvAmt;
	char	FC1ModSource,
			MixerAModAmt,
			FC2ModSource,
			MixerBModAmt;
	WORD	FC1ModAmt,
			FC2ModAmt;
	char	Vol,
			BusSelect,
			AmpModSource,
			PanModSource;
	WORD	XFadeA,
			XFadeB,
			XFadeC,
			XFadeD;
	char	Pan,
			PanFrac,
			AmpModAmt,
			PanModAmt,
			LFOWave,
			LFOBoost,
			LFOSpeed,
			LFORateModAmt;
	WORD	LFODepth;
	char	LFODelay,
			LFOModAmt,
			LFOModSource,
			LFORateModSource;
	WORD	LFOMode,
			RandomModFreq,
			LoopMode;
	WAVEPOSITION SampleStart;			// in chars
	WORD	SampleStartFrac;			// in 256ths of a char (I think...)
	WAVEPOSITION SampleEnd;
	WORD	SampleEndFrac;
	WAVEPOSITION LoopStart;
	WORD	LoopStartFrac;
	WAVEPOSITION LoopEnd;
	WORD	LoopEndFrac,
			SampleRate,					// period = rate * 1.6 usec
			KeyLo,
			KeyHi;
	char	StartLoopModSource,
			DelayModAmt;
	WORD	StartLoopModAmt,
			StartLoopModRange,
			ModType,
			Unused;
} WAVESAMPLEHEADER;

#endif
