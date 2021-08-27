## SafetyLimiter

This is a project to design an audio limiter with the following properties:

- Zero latency, and therefore no lookahead.
- Strictly compresses audio to the [-1, 1] range, no matter how loud or how fast the attack.
- Completely transparent if input audio is already within [-1, 1].
- Nearly colorless, with low THD on sine tones.
- Auto-mutes on NaNs, infinities, and very large inputs to protect your ears and speakers.
- DC removal prior to limiting (optional).
- Header-only C++14 in a single file, `safety_limiter.hpp`.
- Public domain.

The following are the responsibility of the user:

- "True peak" limiting. Use 4x oversampling outside the limiter for that.
