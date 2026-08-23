#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Relatorio das pecas: dimensoes, volume e massa estimada em PETG.

Uso:
    python cad/dims.py            # todas as pecas em ../stl
    python cad/dims.py stl/funil.stl
"""
import glob
import os
import struct
import sys

DENSIDADE = 1.27      # g/cm3 do PETG
APROV = 0.60          # fracao do volume que vira plastico (3 paredes + 20%)


def _tris_ascii(path):
    v = []
    with open(path, 'r', errors='ignore') as f:
        for line in f:
            s = line.lstrip()
            if s.startswith('vertex'):
                p = s.split()
                v.append((float(p[1]), float(p[2]), float(p[3])))
                if len(v) == 3:
                    yield v[0], v[1], v[2]
                    v = []


def _tris_bin(path):
    with open(path, 'rb') as f:
        f.read(80)
        (n,) = struct.unpack('<I', f.read(4))
        for _ in range(n):
            d = f.read(50)
            if len(d) < 50:
                return
            t = struct.unpack('<12f', d[:48])
            yield (t[3], t[4], t[5]), (t[6], t[7], t[8]), (t[9], t[10], t[11])


def triangulos(path):
    with open(path, 'rb') as f:
        cab = f.read(200)
    ascii_stl = cab[:5] == b'solid' and b'facet' in cab
    return _tris_ascii(path) if ascii_stl else _tris_bin(path)


def medir(path):
    xs = ys = zs = None
    vol = 0.0
    n = 0
    for a, b, c in triangulos(path):
        n += 1
        for p in (a, b, c):
            if xs is None:
                xs = [p[0], p[0]]
                ys = [p[1], p[1]]
                zs = [p[2], p[2]]
            else:
                xs[0] = min(xs[0], p[0]); xs[1] = max(xs[1], p[0])
                ys[0] = min(ys[0], p[1]); ys[1] = max(ys[1], p[1])
                zs[0] = min(zs[0], p[2]); zs[1] = max(zs[1], p[2])
        vol += (a[0] * (b[1] * c[2] - c[1] * b[2])
                - b[0] * (a[1] * c[2] - c[1] * a[2])
                + c[0] * (a[1] * b[2] - b[1] * a[2])) / 6.0
    if xs is None:
        return None
    return {
        'tri': n,
        'dx': xs[1] - xs[0], 'dy': ys[1] - ys[0], 'dz': zs[1] - zs[0],
        'cm3': abs(vol) / 1000.0,
    }


def main(argv):
    base = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    alvos = argv[1:]
    if not alvos:
        stl = os.path.join(base, 'stl')
        alvos = sorted(glob.glob(os.path.join(stl, '*.stl')))
        alvos += sorted(glob.glob(os.path.join(stl, '*', '*.stl')))

    print('%-40s %7s %22s %8s %8s' %
          ('peca', 'facetas', 'dimensoes (mm)', 'volume', 'PETG'))
    print('-' * 92)
    comum = 0.0
    por_motor = {}
    erro = 0
    for a in alvos:
        m = medir(a)
        nome = os.path.relpath(a, base).replace(os.sep, '/')
        if not m:
            print('%-40s  VAZIO / ilegivel' % nome)
            erro = 1
            continue
        g = m['cm3'] * DENSIDADE * APROV
        # totaliza por kit: as pecas comuns + a pasta de um motor so.
        # calha_com_sensor e alternativa da calha, nao soma as duas.
        if ('montagem' not in nome and 'peca_de_teste' not in nome
                and 'calha_com_sensor' not in nome):
            if '/motor_' in nome:
                mot = nome.split('/motor_')[1].split('/')[0]
                por_motor[mot] = por_motor.get(mot, 0.0) + g
            else:
                comum += g
        # a mesa da K1 Max e 300x300x300
        aviso = ''
        if max(m['dx'], m['dy']) > 300 or m['dz'] > 300:
            aviso = '  <-- NAO CABE NA MESA!'
            erro = 1
        print('%-40s %7d %6.1f x %6.1f x %6.1f %7.1f cm3 %6.1f g%s' %
              (nome, m['tri'], m['dx'], m['dy'], m['dz'], m['cm3'], g, aviso))
    print('-' * 92)
    print('%-40s %55.1f g' % ('pecas comuns aos dois motores', comum))
    for mot in sorted(por_motor):
        print('%-40s %55.1f g  <-- kit completo com esse motor'
              % ('  + ' + mot, comum + por_motor[mot]))
    print('(sem a peca de teste e sem as vistas de montagem)')
    return erro


if __name__ == '__main__':
    sys.exit(main(sys.argv))
