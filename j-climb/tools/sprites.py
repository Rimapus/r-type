#!/usr/bin/env python3
"""Draw the sprites of J-Climb in its own 16 colors and write them to assets/sprites/.

The pictures are drawn as text below, one character per pixel: edit one, then run
`python3 tools/sprites.py` from the root of the repository. Only the standard library is used.
"""

import struct
import sys
import zlib
from pathlib import Path

# The 16 colors of the game, by the character that draws them; "." is see-through
PALETTE = {
    "0": (0x14, 0x10, 0x1E),  # ink
    "1": (0x1E, 0x22, 0x3C),  # night, the sky behind the tower
    "2": (0x34, 0x3A, 0x60),  # dusk
    "3": (0x4B, 0x58, 0x70),  # dark steel
    "4": (0x7A, 0x89, 0x9E),  # steel
    "5": (0xB6, 0xC3, 0xCF),  # light steel
    "6": (0xF2, 0xEF, 0xDC),  # white
    "7": (0x76, 0x38, 0x2C),  # dark rust
    "8": (0xBC, 0x5A, 0x34),  # rust
    "9": (0xF0, 0xA4, 0x38),  # amber
    "a": (0xFC, 0xDE, 0x68),  # yellow
    "b": (0x2A, 0xB2, 0xA2),  # teal
    "c": (0x8C, 0xE6, 0xBE),  # mint
    "d": (0xDE, 0x3A, 0x4A),  # red
    "e": (0x86, 0x58, 0xC4),  # violet
    "f": (0x5A, 0xB4, 0xEE),  # sky, the rain
}

TILE = 8

# A steel plate of the tower, lit on top, then the girders under it
GROUND_TOP = [
    "55555555",
    "45444454",
    "33333333",
    "33322333",
    "33233233",
    "32333323",
    "23333332",
    "33333333",
]

GROUND = [
    "32333323",
    "33233233",
    "33322333",
    "33322333",
    "33233233",
    "32333323",
    "23333332",
    "33333333",
]

SPIKE_UP = [
    "........",
    "........",
    "........",
    "..6...6.",
    "..5...5.",
    ".545.545",
    "54445444",
    "33333333",
]

SPIKE_DOWN = list(reversed(SPIKE_UP))

# The gear the robot collects, one per room
GEAR = [
    "...aa...",
    ".9aaaa9.",
    ".aa99aa.",
    "aa9..9aa",
    "aa9..9aa",
    ".aa99aa.",
    ".9aaaa9.",
    "...aa...",
]

# The beacon at the top of the tower: touching it ends the run
BEACON = [
    "...aa...",
    "..a66a..",
    "..9aa9..",
    "...55...",
    "...44...",
    "...44...",
    "..3443..",
    ".333333.",
]

BLOCK = ["66666666"] * TILE

# A spring on the ground, at rest then pressed: it throws the robot up
SPRING = [
    "........",
    "........",
    "........",
    ".999999.",
    "..5..5..",
    "...55...",
    "..5..5..",
    ".333333.",
]

SPRING_PRESSED = [
    "........",
    "........",
    "........",
    "........",
    "........",
    ".999999.",
    "..5555..",
    ".333333.",
]

# A rusty block that breaks once the robot stands on it, whole then cracked
CRUMBLE = [
    "88888888",
    "87888878",
    "88878888",
    "87888788",
    "88888878",
    "78878888",
    "88888888",
    "77777777",
]

CRUMBLE_CRACKED = [
    "88808888",
    "87808878",
    "88070888",
    "80888708",
    "88800878",
    "78878088",
    "80888808",
    "77777777",
]

# A moving platform, two tiles wide: the robot stands on its top
PLATFORM = [
    "5555555555555555",
    "4444444444444444",
    "4c444444444444c4",
    "3333333333333333",
    ".33..........33.",
    "................",
    "................",
    "................",
]

# A battery that gives the dash back in the air
REFILL = [
    "...66...",
    "..bbbb..",
    "..bccb..",
    "..bccb..",
    "..bbbb..",
    "..bccb..",
    "..bbbb..",
    "........",
]

# The robot facing right: h is the lamp of its antenna, lit while the dash is ready
ROBOT = {
    "idle": [
        "....h...",
        "....3...",
        ".444444.",
        ".45bbbc.",
        ".444444.",
        "..3553..",
        "..4444..",
        "..3..3..",
    ],
    "run1": [
        "....h...",
        "....3...",
        ".444444.",
        ".45bbbc.",
        ".444444.",
        "..3553..",
        ".44444..",
        ".3...3..",
    ],
    "run2": [
        "........",
        "....h...",
        ".444444.",
        ".45bbbc.",
        ".444444.",
        "..3553..",
        "..4444..",
        "...33...",
    ],
    "jump": [
        "....h...",
        "....3...",
        ".444444.",
        ".45bbbc.",
        ".444444.",
        ".335533.",
        "..4444..",
        ".3....3.",
    ],
}

ROBOT_FRAMES = ["idle", "run1", "run2", "jump"]

# Lit yellow while the dash is ready, off once it is used
LAMP = {"ready": "a", "used": "2"}

# 3x5 characters, in cells of 4x6 so that a text needs no spacing
FONT = {
    " ": ["...", "...", "...", "...", "..."],
    "0": ["###", "#.#", "#.#", "#.#", "###"],
    "1": [".#.", "##.", ".#.", ".#.", "###"],
    "2": ["###", "..#", "###", "#..", "###"],
    "3": ["###", "..#", ".##", "..#", "###"],
    "4": ["#.#", "#.#", "###", "..#", "..#"],
    "5": ["###", "#..", "###", "..#", "###"],
    "6": ["#..", "#..", "###", "#.#", "###"],
    "7": ["###", "..#", "..#", "..#", "..#"],
    "8": ["###", "#.#", "###", "#.#", "###"],
    "9": ["###", "#.#", "###", "..#", "..#"],
    "A": ["###", "#.#", "###", "#.#", "#.#"],
    "B": ["###", "#.#", "##.", "#.#", "###"],
    "C": ["###", "#..", "#..", "#..", "###"],
    "D": ["##.", "#.#", "#.#", "#.#", "##."],
    "E": ["###", "#..", "##.", "#..", "###"],
    "F": ["###", "#..", "##.", "#..", "#.."],
    "G": ["###", "#..", "#.#", "#.#", "###"],
    "H": ["#.#", "#.#", "###", "#.#", "#.#"],
    "I": ["###", ".#.", ".#.", ".#.", "###"],
    "J": ["###", ".#.", ".#.", ".#.", "##."],
    "K": ["#.#", "#.#", "##.", "#.#", "#.#"],
    "L": ["#..", "#..", "#..", "#..", "###"],
    "M": ["###", "###", "#.#", "#.#", "#.#"],
    "N": ["##.", "#.#", "#.#", "#.#", "#.#"],
    "O": ["###", "#.#", "#.#", "#.#", "###"],
    "P": ["###", "#.#", "###", "#..", "#.."],
    "Q": ["###", "#.#", "#.#", "###", "..#"],
    "R": ["###", "#.#", "##.", "#.#", "#.#"],
    "S": ["###", "#..", "###", "..#", "###"],
    "T": ["###", ".#.", ".#.", ".#.", ".#."],
    "U": ["#.#", "#.#", "#.#", "#.#", ".##"],
    "V": ["#.#", "#.#", "#.#", "#.#", ".#."],
    "W": ["#.#", "#.#", "#.#", "###", "###"],
    "X": ["#.#", "#.#", ".#.", "#.#", "#.#"],
    "Y": ["#.#", "#.#", "###", "..#", "###"],
    "Z": ["###", "..#", ".#.", "#..", "###"],
    ":": ["...", ".#.", "...", ".#.", "..."],
    "/": ["..#", ".##", ".#.", "##.", "#.."],
    "!": [".#.", ".#.", ".#.", "...", ".#."],
    ".": ["...", "...", "...", "...", ".#."],
    "-": ["...", "...", "###", "...", "..."],
    ",": ["...", "...", "...", ".#.", "#.."],
    "+": ["...", ".#.", "###", ".#.", "..."],
    ">": ["#..", ".#.", "..#", ".#.", "#.."],
}

# The order of the characters in font.png, also written in assets/scripts/text.lua
FONT_ORDER = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:/!.-,+>"

# The color of the characters of the font, tinted by the scripts
FONT_COLOR = "6"


def blank(width, height):
    return [[None] * width for _ in range(height)]


def paste(image, picture, left, top):
    for y, row in enumerate(picture):
        for x, char in enumerate(row):
            if char != ".":
                image[top + y][left + x] = char


def mirror(picture):
    return [row[::-1] for row in picture]


def write_png(path, image):
    height = len(image)
    width = len(image[0])
    raw = bytearray()
    for row in image:
        raw.append(0)
        for color in row:
            if color is None:
                raw.extend((0, 0, 0, 0))
            else:
                raw.extend((*PALETTE[color], 255))

    def chunk(kind, body):
        return struct.pack(">I", len(body)) + kind + body + struct.pack(">I", zlib.crc32(kind + body))

    data = b"\x89PNG\r\n\x1a\n"
    data += chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0))
    data += chunk(b"IDAT", zlib.compress(bytes(raw), 9))
    data += chunk(b"IEND", b"")
    path.write_bytes(data)


# The pictures of the first row of sheet.png, from the left, as in the FRAME tables of the scripts
TILES = [
    GROUND_TOP, GROUND, SPIKE_UP, SPIKE_DOWN, GEAR, BEACON, BLOCK,
    SPRING, SPRING_PRESSED, CRUMBLE, CRUMBLE_CRACKED, PLATFORM, REFILL,
]


def draw_sheet():
    """The tiles on the first row, then the robot: lamp lit facing right, then left, lamp off right, then left."""
    sheet = blank(TILE * 16, TILE * 5)
    left = 0
    for picture in TILES:
        paste(sheet, picture, left, 0)
        left += len(picture[0])
    rows = [("ready", False), ("ready", True), ("used", False), ("used", True)]
    for row, (lamp, left) in enumerate(rows, start=1):
        for index, name in enumerate(ROBOT_FRAMES):
            picture = [line.replace("h", LAMP[lamp]) for line in ROBOT[name]]
            paste(sheet, mirror(picture) if left else picture, index * TILE, row * TILE)
    return sheet


def draw_font():
    font = blank(4 * len(FONT_ORDER), 6)
    for index, char in enumerate(FONT_ORDER):
        paste(font, [line.replace("#", FONT_COLOR) for line in FONT[char]], index * 4, 0)
    return font


def main():
    folder = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("assets/sprites")
    write_png(folder / "sheet.png", draw_sheet())
    write_png(folder / "font.png", draw_font())


if __name__ == "__main__":
    main()
