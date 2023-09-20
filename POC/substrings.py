from pathlib import Path
from multiprocessing import Pool, cpu_count
from difflib import get_close_matches
from timeit import default_timer as timer
from numba import njit
from math import log

MAX_ENT = 3.5
MIN_ENT = 2.5


class Substrings:

    def __init__(self):
        self.data = None
        self.keys = None

    def process_file(self, path, minl, maxl):
        with open(path, mode='rb') as f:
            self.process(f.read(), minl, maxl)

    @staticmethod
    def slice_file(path, maxl, pool_size, scale=1):
        psize = pool_size * scale
        fsize = Path(path).stat().st_size
        dv, md = divmod(fsize, psize)
        return [(
            i * dv if i == 0 else i * dv - maxl,
            (dv if i == 0 else dv + maxl) if i < psize - 1 else dv + maxl + md
        ) for i in range(psize)]

    def process(self, data, minl, maxl, ascii=False):
        self.keys = {}
        for start in range(len(data) - maxl):
            for length in range(minl, maxl + 1)[::3]:
                subd = data[start:start + length]
                if ascii and not subd.isascii():
                    break
                ent = self.entropy(subd)
                if MAX_ENT > ent > MIN_ENT:
                    self.keys[subd] = self.keys.get(subd, 0) + 1
                else:
                    break

    def top(self, amount):
        self.data = sorted(self.keys.items(), key=lambda i: (i[1], len(i[0])), reverse=True)

        counter = 0
        dit = iter(self.data)
        sims = []
        while counter < amount:
            k, n = next(dit)
            if get_close_matches(k, sims, cutoff=0.8):
                sims.append(k)
                continue
            sims.append(k)
            counter += 1
            yield k, n

    def accumulate(self, subs):
        if self.keys is None:
            self.keys = {}
        for k, v in subs.keys.items():
            if k == 1:
                continue
            if k in self.keys:
                self.keys[k] += v
            else:
                self.keys[k] = v

    @staticmethod
    @njit
    def entropy(data: bytes):
        ent = 0.0

        lfreqs = [0] * 256

        if len(data) < 2:
            return ent

        for i in data:
            lfreqs[i] += 1

        size = float(len(data))
        for i in lfreqs:
            if i > 0:
                freq = i / size
                ent -= freq * log(freq) / log(2.0)
        return ent


def worker(args):
    path, range, minl, maxl, ascii = args
    subs = Substrings()
    with open(path, mode='rb') as f:
        f.seek(range[0])
        data = f.read(range[1])
    subs.process(data, minl, maxl, ascii)
    return subs


def process_all(subs, path, minl, maxl, ascii=False, scale=8):
    cpuc = cpu_count()
    with Pool(cpuc) as pool:
        for i in pool.imap_unordered(
            worker,
            [(path, j, minl, maxl, ascii) for j in subs.slice_file(path, maxl, cpuc, scale)]
        ):
            subs.accumulate(i)


if __name__ == '__main__':
    subs = Substrings()

    start = timer()
    process_all(subs, '<PATH TO FILE>', 15, 30, ascii=False, scale=8)
    end = timer()
    ctime = (end - start)
    print("overall time : ", ctime * 1e3, " ms")

    for k, c in subs.top(30):
        print(k, c)


