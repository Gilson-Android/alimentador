#!/usr/bin/env bash
# =====================================================================
#  Teste de interferencia entre as pecas, na posicao de montagem.
#
#  Renderiza a INTERSECAO de cada par e mede o volume:
#     volume 0        -> as pecas so se TOCAM (apoio, encaixe) = OK
#     volume > 0      -> as pecas OCUPAM O MESMO ESPACO = nao monta
#
#  Uso:  ./check.sh
#  Foi assim que descobri que as orelhas do colar levantadas colidiam com o
#  chanfro do funil, e que a calha batia 0,6mm na barriga do tubo.
# =====================================================================
set -u
OS="${OPENSCAD:-/c/Program Files/OpenSCAD/openscad.com}"
PY="${PYTHON:-python}"
TMP="${TMPDIR:-/tmp}"
FAIL=0

pares="funil_tubo rosca_tubo calha_tubo suporte_tubo funil_suporte"

for m in 28BYJ NEMA17; do
  echo "motor $m:"
  for par in $pares; do
    out="$TMP/int_${m}_${par}.stl"
    "$OS" -o "$out" -D "PAR=\"$par\"" -D "motor_type=\"$m\"" \
          check_interferencia.scad > "$TMP/log.txt" 2>&1
    if grep -qi "top level object is empty" "$TMP/log.txt"; then
      printf '  %-16s sem contato\n' "$par"
      continue
    fi
    vol=$("$PY" - "$out" <<'PY'
import sys, struct
def tris(p):
    with open(p,'rb') as f: head=f.read(200)
    if head[:5]==b'solid' and b'facet' in head:
        v=[]
        for line in open(p,'r',errors='ignore'):
            s=line.lstrip()
            if s.startswith('vertex'):
                q=s.split(); v.append((float(q[1]),float(q[2]),float(q[3])))
                if len(v)==3: yield v; v=[]
    else:
        with open(p,'rb') as f:
            f.read(80); n=struct.unpack('<I',f.read(4))[0]
            for _ in range(n):
                d=f.read(50); t=struct.unpack('<12f',d[:48])
                yield [(t[3],t[4],t[5]),(t[6],t[7],t[8]),(t[9],t[10],t[11])]
V=0.0
for (a,b,c) in tris(sys.argv[1]):
    V += (a[0]*(b[1]*c[2]-c[1]*b[2]) - b[0]*(a[1]*c[2]-c[1]*a[2])
          + c[0]*(a[1]*b[2]-b[1]*a[2]))/6.0
print('%.4f' % (abs(V)/1000.0))
PY
)
    ok=$("$PY" -c "print(1 if float('$vol') < 0.002 else 0)")
    if [ "$ok" = "1" ]; then
      printf '  %-16s OK (so contato, %s cm3)\n' "$par" "$vol"
    else
      printf '  %-16s COLIDE: %s cm3 de sobreposicao\n' "$par" "$vol"
      FAIL=1
    fi
  done
done

if [ "$FAIL" -ne 0 ]; then
  echo "FALHOU: ha pecas ocupando o mesmo espaco (veja acima)." >&2
  exit 1
fi
echo "Todas as pecas encaixam."
