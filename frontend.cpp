#include <iostream>
#include <vector>
#include <memory>
#include <string>

#include <sndfile.h>

#include "safety_limiter.hpp"

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cerr << "Usage: safety_limiter <in file> <out wav> [-a <amplify>]" << std::endl;
        exit(1);
    }

    const char* inFileName = argv[1];
    const char* outFileName = argv[2];
    float amplify = 1;
    if (argc >= 5 && (std::string(argv[3]) == "--amplify" || std::string(argv[3]) == "-a")) {
        amplify = std::pow(10, std::stof(argv[4]) / 20);
    }

    SF_INFO sfInfo;
    sfInfo.format = 0;
    auto soundFile = sf_open(inFileName, SFM_READ, &sfInfo);

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

    std::vector<std::unique_ptr<SafetyLimiter>> limiters;

    for (int i = 0; i < channels; i++) {
        limiters.push_back(std::make_unique<SafetyLimiter>(sampleRate));
    }
    for (int i = 0; i < frames; i++) {
        for (int j = 0; j < channels; j++) {
            int index = i * channels + j;
            audio[index] = limiters[j]->process(audio[index] * amplify);
        }
    }

    SF_INFO sfInfo2;
    sfInfo2.samplerate = sampleRate;
    sfInfo2.channels = 2;
    sfInfo2.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    sfInfo2.sections = 0;
    sfInfo2.seekable = 0;
    auto soundFile2 = sf_open(outFileName, SFM_WRITE, &sfInfo2);
    if (soundFile2 == nullptr) {
        std::cerr << "Audio saving failed: " << sf_strerror(soundFile2);
        exit(1);
    }
    sf_writef_float(soundFile2, audio, frames);
    sf_close(soundFile2);

    delete[] audio;
}
