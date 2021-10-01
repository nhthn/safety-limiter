## SafetyLimiter

This is a project to design an audio limiter with the following properties:

- Zero latency, and therefore no lookahead.
- Strictly compresses audio to the [-1, 1] range, no matter how loud or how fast the attack.
- Completely transparent if input audio is already within [-1, 1].
- Nearly colorless, with low THD tested on sine tones.
- Header-only C++14 in a single file, `safety_limiter.hpp`.
- Public domain.

Limitations and tips:

- DC removal should be done prior to using this plugin.
- Some pumping effects are audible if the limiter is pushed hard.
- Use 4x oversampling outside the limiter for true peak limiting.

### Running tests

Only works on Linux for now.

```
make
pytest test_safety_limiter.py
```
