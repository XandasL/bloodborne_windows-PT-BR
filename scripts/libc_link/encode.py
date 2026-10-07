# SPDX-License-Identifier: GPL-2.0-or-later
"""Module and library identifier encoding."""

ALPHABET = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-'


def encode_id(value):
    """Encode an integer value into a 64-char alphabet identifier."""
    result = ALPHABET[value & 63]
    value >>= 6
    while value:
        result = ALPHABET[value & 63] + result
        value >>= 6
    return result
