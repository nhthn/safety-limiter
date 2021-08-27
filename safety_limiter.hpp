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
    float signalInstantaneousAmplitude = std::abs(sanitize(in));
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
