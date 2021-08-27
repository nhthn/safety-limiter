#include <iostream>
#include <vector>
#include <memory>
#include <string>

#include <sndfile.h>

#include "tclap/CmdLine.h"

#include "safety_limiter.hpp"

int main(int argc, char* argv[])
{
    std::string inFileName;
    std::string outFileName;
    float amplify;

    try {
        TCLAP::CmdLine cmd("safety-limiter frontend", ' ', "0.0.1");

        TCLAP::UnlabeledValueArg<std::string> inFileArg("inFile", "Input sound file", true, "", "string");
        cmd.add(inFileArg);

        TCLAP::UnlabeledValueArg<std::string> outFileArg("outFile", "Output WAV file", true, "", "string");
        cmd.add(outFileArg);

        TCLAP::ValueArg<std::string> amplifyArg("a", "amplify", "Amplification in dB", false, "0", "float");
        cmd.add(amplifyArg);

        cmd.parse(argc, argv);

        inFileName = inFileArg.getValue();
        outFileName = outFileArg.getValue();
        amplify = std::pow(10, std::stof(amplifyArg.getValue()) / 20);
    } catch (TCLAP::ArgException e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        exit(1);
    } catch (std::invalid_argument e) {
        std::cerr << "error: invalid float string specified for --amplify" << std::endl;
        exit(1);
    }

    SF_INFO sfInfo;
    sfInfo.format = 0;
    auto soundFile = sf_open(inFileName.c_str(), SFM_READ, &sfInfo);

    if (soundFile == nullptr) {
        std::cerr << "Audio loading failed: " << sf_strerror(soundFile);
        exit(1);
    }

    int sampleRate = sfInfo.samplerate;
    int frames = sfInfo.frames;
    int channels = sfInfo.channels;
    float* audio = new float[frames * channels];

    sf_readf_float(soundFile, audio, frames);
    sf_close(soundFile);

    std::vector<std::unique_ptr<safety_limiter::SafetyLimiter>> limiters;

    for (int i = 0; i < channels; i++) {
        limiters.push_back(std::make_unique<safety_limiter::SafetyLimiter>(sampleRate));
    }
    for (int i = 0; i < frames; i++) {
        for (int j = 0; j < channels; j++) {
            int index = i * channels + j;
            audio[index] = limiters[j]->process(audio[index] * amplify);
        }
    }

    SF_INFO sfInfo2;
    sfInfo2.samplerate = sampleRate;
    sfInfo2.channels = channels;
    sfInfo2.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    sfInfo2.sections = 1;
    sfInfo2.seekable = 0;
    auto soundFile2 = sf_open(outFileName.c_str(), SFM_WRITE, &sfInfo2);
    if (soundFile2 == nullptr) {
        std::cerr << "Audio saving failed: " << sf_strerror(soundFile2);
        exit(1);
    }
    sf_writef_float(soundFile2, audio, frames);
    sf_close(soundFile2);

    delete[] audio;
}
