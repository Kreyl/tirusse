import pcbnew

x = 136300000
brd = pcbnew.GetBoard()
fpts = brd.GetFootprints()

for fp in fpts:
    ref = fp.GetReference()
    if ref.startswith('Q'):
        fp.SetOrientationDegrees(90)
        N = int(ref.removeprefix('Q'))
        if 2 <= N <= 38:
            fp.SetX(x)
            fp.SetY(264100000 - (N - 2) * 7100000)
        elif 39 <= N <= 73:
            fp.Flip(pcbnew.VECTOR2I(0,0), True)
            fp.SetX(136300000)
            fp.SetY(264100000 - (74 - N) * 7100000)
        elif N == 74:
            fp.Flip(pcbnew.VECTOR2I(0,0), True)
            fp.SetX(x)
            fp.SetY(297800000)

for fp in fpts:
    ref = fp.GetReference()
    if ref.startswith('C'):
        fp.SetOrientationDegrees(-90)
        N = int(ref.removeprefix('C'))
        if 28 <= N <= 64:
            fp.SetX(136800000)
            fp.SetY(267500000 - (N - 28) * 7100000)

for fp in fpts:
    ref = fp.GetReference()
    if ref.startswith('C'):
        fp.SetOrientationDegrees(-90)
        N = int(ref.removeprefix('C'))
        if 64 <= N <= 100:
            fp.Flip(pcbnew.VECTOR2I(0,0), True)
            fp.SetX(136800000)
            fp.SetY(267500000 - (99 - N) * 7100000)

            for fp in fpts:
                ref = fp.GetReference()
                if ref.startswith('Q'):
                    fp.SetOrientationDegrees(90)
                    N = int(ref.removeprefix('Q'))
                    if N == 2: