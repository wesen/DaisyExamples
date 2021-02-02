#include "daisysp.h"
#include "daisy_pod.h"

using namespace daisysp;
using namespace daisy;

static DaisyPod pod;

static Tone tone;
static Parameter cutoffParam;
static Parameter debugParam;
float cutoff;
float debug_ = 0;

//Helper functions
void Controls();

void GetCrushSample(float &outl, float &outr, float inl, float inr);

class EnvFollower
{
public:
    EnvFollower() = default;

    void Init(float sample_rate) {
        _sample_rate = sample_rate;
        SetA(0);
        SetR(0.5f);
        _z_1 = 0.0f;
    }

    float Process(float &inl, float &inr)
    {
        float x = (abs(inl) + abs(inr)) * 0.5f - _z_1;
        x *= (x  > 0 ? _wplus : _wminus);
        x += _z_1;
        _z_1 = x;
        return x;
    }

    void SetA(float a_s)
    {
        _a_s = a_s;
        _wplus = time2Cutoff(_a_s);
    }

    void SetR(float r_s)
    {
        _r_s = r_s;
        _wminus = time2Cutoff(_r_s);
    }

protected:
    inline float time2Cutoff(float &t) {
        float a = -1.0f / daisysp::fmax(t * _sample_rate, 0.02f);
        return 1.0f - exp2f(a);
    }

    float _sample_rate;
    float _a_s;
    float _r_s;
    float _z_1;
    float _wplus, _wminus;
};

EnvFollower envF;

void AudioCallback(float *in, float *out, size_t size)
{
    float outl, outr, inl, inr;

    Controls();

    float env = 0;

    //audio
    for (size_t i = 0; i < size; i += 2) {
        inl = in[i];
        inr = in[i + 1];

        outl = tone.Process(inl);
        outr = tone.Process(inr);

        env += envF.Process(inl, inr);

        // left out
        out[i] = outl;

        // right out
        out[i + 1] = outr;
    }

    env /= (size / 2.0f);
    env *= 1.f;
    pod.led1.Set(env, 0, 0);
    pod.led2.Set(env, 0, 0);
    pod.UpdateLeds();
}

int main(void)
{
    // initialize pod hardware and oscillator daisysp module
    float sample_rate;

    //Inits and sample rate
    pod.Init();
    sample_rate = pod.AudioSampleRate();
    tone.Init(sample_rate);
    envF.Init(sample_rate);

    //set parameters
    cutoffParam.Init(pod.knob1, 500, 20000, cutoffParam.LOGARITHMIC);
    debugParam.Init(pod.knob2, 1, 100, debugParam.LINEAR);

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
    debug_ = debugParam.Process();
    tone.SetFreq(cutoff);
}

void UpdateEncoder()
{
    pod.encoder.Increment();
}

void UpdateLeds(float k1, float k2)
{
//    pod.led1.Set(k1, 0, 0);
//    pod.led2.Set(k2, 0, 0);
//
//    pod.UpdateLeds();
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

