import argparse
import sys


def get_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        prog="swap_raw",
        description="Swaps around the bytes of a raw sample file.",
        epilog="Little Endian (x86/_64) is converted to Big Endian (m68k) "
        "and vice versa.",
    )
    parser.add_argument(
        "-sw",
        "--sample_width",
        help="Widths of a single sample in bits.",
        required=True,
        type=int,
    )
    parser.add_argument(
        "-i", "--file_in", help="File to be read and converted.", required=True
    )
    parser.add_argument(
        "-o", "--file_out", help="File to be written finally.", required=True
    )
    args = parser.parse_args()

    return args


def swap(byte_widths: int, file_in: str, file_out: str) -> int:

    try:
        with open(file_in, "rb") as f_in:
            try:
                with open(file_out, "wb") as f_out:
                    while True:
                        b = f_in.read(byte_widths)
                        # print(len(b))
                        if not b:
                            break
                        if len(b) != byte_widths:
                            print(
                                "WARNING: File size is not a multiple of "
                                f"sample widths {b} bytes!"
                            )
                        else:
                            mem = memoryview(b)
                            f_out.write(mem[-1].to_bytes())
                            if 3 == byte_widths:
                                f_out.write(mem[1].to_bytes())
                            f_out.write(mem[0].to_bytes())
                    return 0
            except IOError as error:
                print(f"ERROR: Could not write file {file_out}!\nReason: {error}")
                return -3
    except IOError as error:
        print(f"ERROR: Could not read file {file_in}!\nReason: {error}")
        return -2


def main() -> int:
    args = get_args()

    byte_widths = args.sample_width / 8
    if byte_widths not in [2, 3]:
        print(f"Nothing to do for sample width of {args.sample_width} bits.")
        return -1

    result = swap(int(byte_widths), args.file_in, args.file_out)
    return result


if __name__ == "__main__":
    sys.exit(main())
