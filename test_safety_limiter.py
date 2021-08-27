import numpy as np
import pytest

from safety_limiter import apply_limiter, measure_thd


def sine_wave(frequency=440, duration=1.0, sample_rate=48000.0):
    t = np.arange(0, int(duration * sample_rate)) / sample_rate
    signal = np.sin(t * 2 * np.pi * frequency)
    return signal


class TestLimiter:
    def test_zero(self):
        """It passes through silence."""
        signal = np.zeros(1000)
        out = apply_limiter(signal, sample_rate=48000.0)["out"]
        np.testing.assert_allclose(signal, out)

    def test_under_threshold(self):
        """It passes through a signal with peak amplitude less than 1."""
        sample_rate = 48000.0
        signal = sine_wave(sample_rate=sample_rate) * 0.5
        out = apply_limiter(signal, sample_rate=sample_rate)["out"]
        np.testing.assert_allclose(signal, out)

    def test_over_threshold(self):
        """If the peak amplitude of the input is greater than 1, the peak amplitude of the
        output is less than 1."""
        sample_rate = 48000.0
        signal = sine_wave(sample_rate=sample_rate) * 1.2
        out = apply_limiter(signal, sample_rate=sample_rate)["out"]
        assert np.max(np.abs(out)) <= 1

    def test_no_dips(self):
        """When an amplified sine wave is provided, there are no "cracks" or "dips" in the
        amplitude/gain signal after the initial attack."""
        sample_rate = 48000.0
        frequency = 440
        signal = sine_wave(frequency, duration=1, sample_rate=sample_rate) * 1.2
        amplitude = apply_limiter(signal, sample_rate=sample_rate)["amplitude"]
        mode = "attack"
        last_x = 1
        for x in amplitude:
            if mode == "attack":
                if x == last_x:
                    mode = "hold"
            else:
                assert x == last_x

    def test_thd_static(self):
        """An amplified sine wave has less than 0.001% THD."""
        sample_rate = 48000.0
        frequency = 440
        signal = sine_wave(frequency, sample_rate=sample_rate) * 8
        out = apply_limiter(signal, sample_rate=sample_rate)["out"]
        partials = 8
        thd = (
            measure_thd(out, sample_rate, frequency, partials)
            - measure_thd(signal, sample_rate, frequency, partials)
        )
        assert thd < 0.001

    def test_thd_decay(self):
        """An amplified sine wave with a decaying envelope has less than 0.03% THD."""
        sample_rate = 48000.0
        frequency = 440
        signal = sine_wave(frequency, sample_rate=sample_rate)
        signal *= np.linspace(1, 2, len(signal)) * 8
        out = apply_limiter(signal, sample_rate=sample_rate)["out"]
        partials = 8
        thd = (
            measure_thd(out, sample_rate, frequency, partials)
            - measure_thd(signal, sample_rate, frequency, partials)
        )
        assert thd < 0.03

    def test_thd_attack(self):
        """An amplified sine wave with an increasing envelope has less than 0.1% THD."""
        sample_rate = 48000.0
        frequency = 440
        signal = sine_wave(frequency, sample_rate=sample_rate)
        signal *= np.linspace(2, 1, len(signal)) * 8
        out = apply_limiter(signal, sample_rate=sample_rate)["out"]
        partials = 8
        thd = (
            measure_thd(out, sample_rate, frequency, partials)
            - measure_thd(signal, sample_rate, frequency, partials)
        )
        assert thd < 0.1
