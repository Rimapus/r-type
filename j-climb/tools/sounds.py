#!/usr/bin/env python3
"""Make the sounds and the music of J-Climb and write them to assets/sounds/ and assets/music/.

Every sound is a few square, triangle or noise waves, described below: change one, then run
`python3 tools/sounds.py` from the root of the repository. Only the standard library is used.
"""

import math
import random
import struct
import sys
import wave
from pathlib import Path

RATE = 22050
# The noise is the same each time the script runs, so the files only change when a sound does
NOISE = random.Random(42)


def square(phase):
    return 1.0 if phase % 1.0 < 0.5 else -1.0


def triangle(phase):
    return 4.0 * abs(phase % 1.0 - 0.5) - 1.0


def noise(_phase):
    return NOISE.uniform(-1.0, 1.0)


def sweep(wave_function, start, end, seconds, volume=0.5, decay=True):
    """A wave whose pitch goes from start to end hertz, fading out when decay is set."""
    samples = []
    phase = 0.0
    count = int(seconds * RATE)
    for index in range(count):
        progress = index / count
        phase += (start + (end - start) * progress) / RATE
        envelope = (1.0 - progress) if decay else 1.0
        samples.append(wave_function(phase) * volume * envelope)
    return samples


def notes(wave_function, frequencies, seconds, volume=0.5):
    """Notes one after the other, each one fading out."""
    samples = []
    for frequency in frequencies:
        samples += sweep(wave_function, frequency, frequency, seconds, volume)
    return samples


def mix(*tracks):
    length = max(len(track) for track in tracks)
    return [sum(track[index] for track in tracks if index < len(track)) for index in range(length)]


def note(name):
    """The frequency of a note like "C5" or "F#4"."""
    semitones = {"C": -9, "C#": -8, "D": -7, "D#": -6, "E": -5, "F": -4, "F#": -3, "G": -2, "G#": -1, "A": 0,
                 "A#": 1, "B": 2}
    return 440.0 * 2 ** ((semitones[name[:-1]] + 12 * (int(name[-1]) - 4)) / 12)


SOUNDS = {
    "jump": lambda: sweep(square, 300, 620, 0.12, 0.35),
    "walljump": lambda: mix(sweep(square, 420, 820, 0.12, 0.35), sweep(noise, 0, 0, 0.04, 0.3)),
    "dash": lambda: mix(sweep(noise, 0, 0, 0.18, 0.4), sweep(square, 900, 300, 0.18, 0.2)),
    "break": lambda: mix(sweep(noise, 0, 0, 0.4, 0.5), sweep(square, 400, 60, 0.4, 0.3)),
    "gear": lambda: notes(square, [note("C6"), note("G6")], 0.1, 0.3),
    "spring": lambda: sweep(lambda phase: square(phase) * (1 + 0.5 * math.sin(phase / 6)), 180, 720, 0.25, 0.3),
    "battery": lambda: notes(triangle, [note("E5"), note("G#5"), note("B5")], 0.05, 0.5),
    "crumble": lambda: sweep(noise, 0, 0, 0.2, 0.25),
    "beacon": lambda: notes(square, [note("C5"), note("E5"), note("G5"), note("C6")], 0.16, 0.3)
    + sweep(square, note("C6"), note("C6"), 0.5, 0.3),
    "select": lambda: sweep(square, 880, 880, 0.05, 0.25),
    "confirm": lambda: notes(square, [note("A5"), note("E6")], 0.06, 0.25),
}

# The music of the run: a melody on a square wave over a triangle bass, a bar of four beats per line
TEMPO = 140
MELODY = [
    "E5 G5 A5 G5", "E5 D5 C5 D5", "E5 G5 A5 C6", "B5 A5 G5 -",
    "E5 G5 A5 G5", "E5 D5 C5 D5", "E5 D5 C5 B4", "A4 - - -",
]
BASS = ["A2", "F2", "C3", "G2", "A2", "F2", "G2", "A2"]


def make_music():
    beat = 60.0 / TEMPO
    melody = []
    bass = []
    for bar, chord in zip(MELODY, BASS):
        for name in bar.split():
            if name == "-":
                melody += [0.0] * int(beat * RATE)
            else:
                melody += sweep(square, note(name), note(name), beat, 0.18)
        for _ in range(4):
            bass += sweep(triangle, note(chord), note(chord), beat, 0.35, decay=False)[: int(beat * RATE)]
    hats = []
    for _ in range(len(MELODY) * 8):
        hats += sweep(noise, 0, 0, beat / 2, 0.06)[: int(beat / 2 * RATE)]
    return mix(melody, bass, hats)


def write_wav(path, samples):
    with wave.open(str(path), "wb") as file:
        file.setnchannels(1)
        file.setsampwidth(2)
        file.setframerate(RATE)
        file.writeframes(b"".join(struct.pack("<h", int(max(-1.0, min(1.0, s)) * 32767)) for s in samples))


def main():
    root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("assets")
    for name, make in SOUNDS.items():
        write_wav(root / "sounds" / f"{name}.wav", make())
    write_wav(root / "music" / "climb.wav", make_music())


if __name__ == "__main__":
    main()
