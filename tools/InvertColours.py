# Flips the palette indices in tile data
# Arg 1 = File Path
import sys

inverted = [0b11, 0b10, 0b01, 0b00]

filePath = sys.argv[1]

with open(filePath, "rb") as f:
    fileData = f.read()

with open(filePath, "wb+") as f:
    for byte in fileData:
        newbyte = 0
        newbyte |= inverted[(byte >> 6) & 0b11] << 6
        newbyte |= inverted[(byte >> 4) & 0b11] << 4
        newbyte |= inverted[(byte >> 2) & 0b11] << 2
        newbyte |= inverted[(byte >> 0) & 0b11] << 0
        f.write((newbyte).to_bytes(1, byteorder='big'))
