/*
safety_limiter.hpp -- an audio limiter

https://github.com/nhthn/safety-limiter

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

#pragma once
#include <cmath>
#include <limits>

namespace safety_limiter {

float sanitize(float x) {
    if (std::isinf(x)) {
        return 0;
    }
    if (std::isnan(x)) {
        return 0;
    }
    if (std::abs(x) < std::numeric_limits<float>::min()) {
        return 0;
    }
    return x;
}

class SafetyLimiter {
public:
    SafetyLimiter(float sampleRate);

    void setReleaseTime(float releaseTime);
    void setHoldTime(float holdTime);

    float process(float in);
    float processSidechain(float in, float sidechain);

private:
    const float m_sampleRate;
    float m_kRelease;
    int m_holdTime;
    float m_lastAmplitude = 0.f;
    int m_holdTimer = 0;
};

SafetyLimiter::SafetyLimiter(float sampleRate)
    : m_sampleRate(sampleRate)
{
    setReleaseTime(0.1);
    setHoldTime(0.1);
}

void SafetyLimiter::setReleaseTime(float releaseTime)
{
    if (releaseTime == 0) {
        m_kRelease = 0;
    } else {
        m_kRelease = std::pow(0.001f, 1 / (releaseTime * m_sampleRate));
    }
}

void SafetyLimiter::setHoldTime(float holdTime)
{
    m_holdTime = m_sampleRate * holdTime;
}

float SafetyLimiter::process(float in)
{
    return processSidechain(in, in);
}

float SafetyLimiter::processSidechain(float in, float sidechain)
{
    float signalInstantaneousAmplitude = std::abs(sanitize(sidechain));
    float amplitudeFromFollower;
    if (m_holdTimer > 0) {
        amplitudeFromFollower = m_lastAmplitude;
        m_holdTimer -= 1;
    } else {
        amplitudeFromFollower = (
            m_lastAmplitude * m_kRelease
            + signalInstantaneousAmplitude * (1 - m_kRelease)
        );
    }
    float amplitude;
    if (amplitudeFromFollower > signalInstantaneousAmplitude) {
        amplitude = amplitudeFromFollower;
    } else {
        amplitude = signalInstantaneousAmplitude;
        m_holdTimer = m_holdTime;
    }
    m_lastAmplitude = sanitize(amplitude);

    float gain = amplitude > 1 ? 1 / amplitude : 1;
    return in * gain;
}

} // namespace safety_limiter
