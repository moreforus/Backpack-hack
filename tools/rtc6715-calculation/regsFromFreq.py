#!/usr/bin/python

IntermediateFreq = 480
FreqRefMHz = 8

def main():
    print("Freq:")
    freqInput = float(input())
    freq = (freqInput - IntermediateFreq) / 2

    R = FreqRefMHz
    remainder = freq - int(freq)

    if (remainder > 0):
        if (remainder > 0.5):
            remainder = 1.0 - remainder
        R = round(R / remainder)

    freq = freq * R / FreqRefMHz
    N = int(freq / 32)
    A = int(freq - N * 32)

    if (R < 0 or R >= 2 ** 15):
        print("Error: N=%d A=%d <R=%d>" %(N, A, R))
        exit(1)
    if (A < 0 or A >= 2 ** 7):
        print("Error: N=%d <A=%d> R=%d" %(N, A, R))
        exit(1)
    if (N < 0 or N >= 2 ** 13):
        print("Error: <N=%d> A=%d R=%d" %(N, A, R))
        exit(1)

    Fout = 2 * (N * 32 + A) * FreqRefMHz / R + IntermediateFreq
    Error = freqInput - Fout
    Percents = Error * 100 / freqInput

    print("F=%f error=%f (%f%%)" %(Fout, Error, Percents))
    print("N=%d A=%d R=%d" %(N, A, R))

    data = ((R & 0x7fff) << 5) | 0x10 | 0x00
    print("REG A: %06x" %(data))

    data = ((((N & 0x1fff) << 7) | A) << 5) | 0x10 | 0x01;
    print("REG B: %06x" %(data))

if __name__ == '__main__':
    try:
        main()
    except AssertionError as e:
        print(e)
        exit(1)
