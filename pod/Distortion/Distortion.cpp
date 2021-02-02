#include "daisysp.h"
#include "daisy_pod.h"

using namespace daisysp;
using namespace daisy;

static DaisyPod pod;

static Tone tone;
static Parameter cutoffParam;
float cutoff;

//Helper functions
void Controls();

void GetCrushSample(float &outl, float &outr, float inl, float inr);


void AudioCallback(float *in, float *out, size_t size)
{
    float outl, outr, inl, inr;

    Controls();

    //audio
    for (size_t i = 0; i < size; i += 2) {
        inl = in[i];
        inr = in[i + 1];

        outl = tone.Process(inl);
        outr = tone.Process(inr);

        // left out
        out[i] = outl;

        // right out
        out[i + 1] = outr;
    }
}

int main(void)
{
    // initialize pod hardware and oscillator daisysp module
    float sample_rate;

    //Inits and sample rate
    pod.Init();
    sample_rate = pod.AudioSampleRate();
    tone.Init(sample_rate);

    //set parameters
    cutoffParam.Init(pod.knob1, 500, 20000, cutoffParam.LOGARITHMIC);

    // start callback
    pod.StartAdc();
    pod.StartAudio(AudioCallback);

    while (1) {}
}

void UpdateKnobs(float &k1, float &k2)
{
    k1 = pod.knob1.Process();
    k2 = pod.knob2.Process();

    cutoff = cutoffParam.Process();
    tone.SetFreq(cutoff);
}

void UpdateEncoder()
{
    pod.encoder.Increment();
}

void UpdateLeds(float k1, float k2)
{
    pod.led1.Set(k1, 0, 0);
    pod.led2.Set(k2, 0, 0);

    pod.UpdateLeds();
}

void Controls()
{
    float k1, k2;

    pod.ProcessAnalogControls();
    pod.ProcessDigitalControls();

    UpdateKnobs(k1, k2);

    UpdateEncoder();

    UpdateLeds(k1, k2);
}

