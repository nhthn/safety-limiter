import numpy as np
import scipy.signal

NEGATIVE_60_DBFS = 0.001

def apply_limiter(signal, sample_rate, release_time, hold_time):
    last_amplitude = 0
    amplitude = np.zeros(len(signal))
    out = np.zeros(len(signal))
    gain = np.zeros(len(signal))
    hold_timer = 0
    k_release = NEGATIVE_60_DBFS ** (1 / (release_time * sample_rate))
    for i in range(len(signal)):
        signal_instantaneous_amplitude = abs(signal[i])
        if hold_timer > 0:
            amplitude_from_follower = last_amplitude
            hold_timer -= 1
        else:
            amplitude_from_follower = (
                last_amplitude * k_release
                + signal_instantaneous_amplitude * (1 - k_release)
            )
        if amplitude_from_follower > signal_instantaneous_amplitude:
            current_amplitude = amplitude_from_follower
        else:
            current_amplitude = signal_instantaneous_amplitude
            hold_timer = int(sample_rate * hold_time)
        amplitude[i] = current_amplitude
        last_amplitude = current_amplitude

        if current_amplitude > 1.0:
            gain[i] = 1 / current_amplitude
        else:
            gain[i] = 1
        out[i] = signal[i] * gain[i]

    return {
        "amplitude": amplitude,
        "out": out,
        "gain": gain,
    }

def measure_thd(signal, sample_rate, f0, partials):
    if f0 * partials >= sample_rate * 0.5:
        raise ValueError("Partials exceed Nyquist frequency")

    fft_size = 2048
    hop_size = fft_size // 2

    num_frames = (len(signal) - fft_size) // hop_size + 1
    thd = np.zeros(num_frames)

    for i, offset in enumerate(range(0, len(signal) - fft_size, hop_size)):
        window = scipy.signal.windows.get_window("hann", fft_size, True)
        frame = signal[offset:offset + fft_size] * window
        magnitude_spectrum = np.abs(np.fft.rfft(frame))
        frequencies = np.fft.rfftfreq(fft_size, 1 / sample_rate)

        amplitudes = np.zeros(partials)
        for partial in range(1, partials + 1):
            frequency = f0 * partial
            for index, x in enumerate(frequencies):
                if x > frequency:
                    break
            bin_radius = 2
            low_bin = index - bin_radius
            high_bin = index + bin_radius
            amplitude = np.sqrt(np.sum(np.square(magnitude_spectrum[low_bin:high_bin])))
            amplitudes[partial - 1] = amplitude

        thd[i] = 100 * np.sqrt(
            np.sum(np.square(amplitudes[1:])) / np.square(amplitudes[0])
        )

    return np.mean(thd)


if __name__ == "__main__":
    import matplotlib.pyplot as plt

    sample_rate = 48000
    duration = 1
    t = np.arange(0, int(duration * sample_rate)) / sample_rate
    signal = np.sin(t * 2 * np.pi * 440) * 8

    limited_signal = apply_limiter(
        signal,
        sample_rate=sample_rate,
        hold_time=0.5,
        release_time=0.01,
    )

    print(measure_thd(signal, sample_rate, 440, 7))
    print(measure_thd(limited_signal["out"], sample_rate, 440, 7))

    plt.plot(t, signal)
    plt.plot(t, limited_signal["out"])
    plt.plot(t, limited_signal["amplitude"])
    plt.show()
