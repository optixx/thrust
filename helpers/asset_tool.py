#!/usr/bin/env python3
"""Unified helper for building game assets."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys


def read_bytes(source: str) -> bytes:
    if source == "-":
        return sys.stdin.buffer.read()
    return Path(source).read_bytes()


def write_bytes(destination: str, data: bytes) -> None:
    if destination == "-":
        sys.stdout.buffer.write(data)
    else:
        Path(destination).write_bytes(data)


def pal_to_bin(args: argparse.Namespace) -> int:
    data = read_bytes(args.input)
    if args.skip > len(data):
        raise SystemExit("skip value exceeds file size")
    payload = data[args.skip :]
    write_bytes(args.output, payload)
    return 0


def bmp_to_bin(args: argparse.Namespace) -> int:
    data = read_bytes(args.input)
    if args.skip > len(data):
        raise SystemExit("skip value exceeds file size")
    payload = data[args.skip :]
    if args.block_size <= 0:
        raise SystemExit("block size must be positive")
    if len(payload) % args.block_size:
        raise SystemExit("bitmap payload length is not a multiple of the block size")
    blocks = len(payload) // args.block_size
    writer = open(args.output, "wb")
    try:
        for index in range(blocks - 1, -1, -1):
            start = index * args.block_size
            writer.write(payload[start : start + args.block_size])
    finally:
        writer.close()
    return 0


def format_c_array(var_name: str, data: bytes, chunk_size: int = 8) -> str:
    if chunk_size <= 0:
        raise SystemExit("chunk size must be positive")
    lines = []
    for offset in range(0, len(data), chunk_size):
        chunk = data[offset : offset + chunk_size]
        formatted = ", ".join(f"0x{byte:02x}" for byte in chunk)
        lines.append(f"\t{formatted}")
    array_body = ",\n".join(lines)
    if array_body:
        array_body = f"{array_body}\n"
    return f"\nunsigned char {var_name}[] = {{\n{array_body}}};\n"


def bin_to_c(args: argparse.Namespace) -> int:
    data = read_bytes(args.input)
    output = format_c_array(args.var, data, chunk_size=args.chunk_size)
    sys.stdout.write(output)
    return 0


def sound_to_c(args: argparse.Namespace) -> int:
    data = read_bytes(args.input)
    len_name = args.len_name or f"{args.var}_len"
    sys.stdout.write(f"unsigned int {len_name} = {len(data)};\n")
    sys.stdout.write(format_c_array(args.var, data, chunk_size=args.chunk_size))
    return 0


def escape_line(line: str) -> str:
    replacements = {
        "\t": "\\t",
        "\b": "\\b",
        "\f": "\\f",
        "'": "\\'",
        '"': "\\\"",
        "\\": "\\\\",
    }
    return "".join(replacements.get(ch, ch) for ch in line)


def def_to_c(args: argparse.Namespace) -> int:
    raw_lines = read_bytes(args.input).decode("utf-8", errors="replace").splitlines()
    sys.stdout.write("\n#include <stddef.h>\n\n")
    sys.stdout.write(f"char *{args.var}[] = {{\n")
    for line in raw_lines:
        escaped = escape_line(line)
        sys.stdout.write(f"\"{escaped}\",\n")
    sys.stdout.write("NULL")
    sys.stdout.write(" };\n")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="Asset build helper.")
    parser.add_argument("--debug", action="store_true", help="print debug info")
    subparsers = parser.add_subparsers(dest="command", required=True)

    pal = subparsers.add_parser("pal2bin", help="Convert palette files to binary data")
    pal.add_argument("input")
    pal.add_argument("output")
    pal.add_argument("--skip", type=int, default=790)
    pal.set_defaults(func=pal_to_bin)

    bmp = subparsers.add_parser("bmp2bin", help="Convert bitmap slices to binary data")
    bmp.add_argument("input")
    bmp.add_argument("output")
    bmp.add_argument("--block-size", type=int, required=True)
    bmp.add_argument("--skip", type=int, default=1078)
    bmp.set_defaults(func=bmp_to_bin)

    bin2c = subparsers.add_parser("bin2c", help="Emit a C array from a binary file")
    bin2c.add_argument("var")
    bin2c.add_argument("input")
    bin2c.add_argument("--chunk-size", type=int, default=8)
    bin2c.set_defaults(func=bin_to_c)

    snd = subparsers.add_parser("sound2c", help="Emit a C sound array with length")
    snd.add_argument("var")
    snd.add_argument("input")
    snd.add_argument("--len-name")
    snd.add_argument("--chunk-size", type=int, default=8)
    snd.set_defaults(func=sound_to_c)

    text = subparsers.add_parser("def2c", help="Emit a C string array from a DEF file")
    text.add_argument("var")
    text.add_argument("input")
    text.set_defaults(func=def_to_c)

    args = parser.parse_args()
    if args.debug:
        print(f"Running {args.command} with {args}")
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
