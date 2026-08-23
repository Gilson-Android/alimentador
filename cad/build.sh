#!/usr/bin/env bash
# =====================================================================
#  Gera todos os STL em ../stl
#  Uso (Git Bash no Windows):
#     ./build.sh                 -> todas as pecas
#     ./build.sh --montagem      -> tambem as vistas de conjunto (~12 min)
#  Precisa do OpenSCAD instalado. Ajuste OS abaixo se estiver em outro
#  caminho (Linux/macOS: OS=openscad).
# =====================================================================
set -u
OS="${OPENSCAD:-/c/Program Files/OpenSCAD/openscad.com}"
ROOT=../stl

# render <pasta_saida> <arquivo.scad> <saida.stl> [args extras]
render() {
  local dir="$1"; local src="$2"; local dst="$3"; shift 3
  mkdir -p "$dir"
  printf '  %-14s -> %s\n' "$src" "${dir#../stl/}/$dst"
  "$OS" -o "$dir/$dst" "$@" "$src" 2>&1 | grep -Ei 'error|warning' && return 1
  return 0
}

echo "Pecas iguais para qualquer motor:"
render $ROOT hopper.scad     hopper.stl
render $ROOT lid.scad        lid.stl
render $ROOT chute.scad      chute.stl
render $ROOT chute.scad      chute_com_sensor.stl -D 'sensor_holes=true'
render $ROOT bracket.scad    bracket.stl
render $ROOT saddle_cap.scad saddle_cap.stl
render $ROOT knob.scad       knob.stl
render $ROOT box_base.scad   box_base.stl
render $ROOT box_lid.scad    box_lid.stl
render $ROOT cam_pod.scad    cam_pod.stl

# ---------------------------------------------------------------------
# Pecas que dependem do motor. Imprima SO a pasta do seu motor.
for m in 28BYJ NEMA17; do
  d=$ROOT/motor_$(echo $m | tr 'A-Z' 'a-z')
  echo "Pecas do motor $m:"
  render $d test_fit.scad test_fit.stl               -D "motor_type=\"$m\""
  render $d auger.scad    auger.stl                  -D "motor_type=\"$m\""
  render $d barrel.scad   barrel_a.stl -D 'PART="A"' -D "motor_type=\"$m\""
  render $d barrel.scad   barrel_b.stl -D 'PART="B"' -D "motor_type=\"$m\""
done

# ---------------------------------------------------------------------
# Conjunto montado, SO PARA VISUALIZAR (nao e para imprimir).
# Leva ~6 min cada um, por isso so roda com: ./build.sh --montagem
if [ "${1:-}" = "--montagem" ]; then
  echo "Vistas de conjunto (demora):"
  render $ROOT assembly.scad montagem_completa.stl  --export-format binstl -D 'SHOW_TANK=false'
  render $ROOT assembly.scad montagem_explodida.stl --export-format binstl -D 'EXPLODE=1' -D 'SHOW_TANK=false'
fi

echo "Pronto. STLs em $(cd $ROOT && pwd)"
