/*
 * Engine.h
 * 
 * Tiffany Liu
 * 
 * 9 February 2026
*/


#define EFFECT_SELECTION_MAX       7
#define TREMOLO_SELECTION_MAX      3
#define DELAY_SELECTION_MAX        2
#define REVERB_SELECTION_MAX       2
#define BITCRUSH_SELECTION_MAX     2
#define OVERDRIVE_SELECTION_MAX    2
#define DISTORTION_SELECTION_MAX   2
#define FUZZ_SELECTION_MAX         2


// ============================================================
// [VARIABLES]


enum MenuMode {
    EFFECT_SELECTION_MODE = 0,   // User is choosing from effects
    AUDIO_PARAM_SELECTION_MODE,  // User is choosing parameters of an effect
    AUDIO_PARAM_VALUE_MODE       // User is adjusting a parameter value
};

enum EffectSelection {
    NO_EFFECT = 0,
    TREMOLO,
    DELAY,
    REVERB,
    BITCRUSH,
    OVERDRIVE,
    DISTORTION,
    FUZZ
};

enum TremoloSelection {
    TREMOLO_FREQ = 0, 
    TREMOLO_DEPTH,
    TREMOLO_PHASE,
    TREMOLO_BACK
};
    
enum DelaySelection {
    DELAY_MS = 0,
    DELAY_FEEDBACK,
    DELAY_BACK
};

enum ReverbSelection {
    REVERB_TAPS = 0,
    REVERB_DECAY,
    REVERB_BACK
};

enum BitcrushSelection {
    BITCRUSH_DOWNSAMPLE = 0,
    BITCRUSH_DEPTH,
    BITCRUSH_BACK
};

enum OverdriveSelection {
    OVERDRIVE_DRIVE = 0,
    OVERDRIVE_TONE,
    OVERDRIVE_BACK
};

enum DistortionSelection {
    DISTORTION_DRIVE = 0,
    DISTORTION_TONE,
    DISTORTION_BACK
};

enum FuzzSelection {
    FUZZ_DRIVE = 0,
    FUZZ_TONE,
    FUZZ_BACK
};

struct AudioParamSelection {
    TremoloSelection    TREMOLO    = TREMOLO_FREQ;
    DelaySelection      DELAY      = DELAY_MS;
    ReverbSelection     REVERB     = REVERB_TAPS;
    BitcrushSelection   BITCRUSH   = BITCRUSH_DOWNSAMPLE;
    OverdriveSelection  OVERDRIVE  = OVERDRIVE_DRIVE;
    DistortionSelection DISTORTION = DISTORTION_DRIVE;
    FuzzSelection       FUZZ       = FUZZ_DRIVE;
};

extern MenuMode            MENU_MODE;
extern EffectSelection     EFFECT_SELECTION;
extern AudioParamSelection AUDIO_PARAM_SELECTION;


// ============================================================
// [FUNCTIONS]

// put this in the engine class
void readPeripherals(void);


