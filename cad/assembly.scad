// =====================================================================
//  AquaFeeder -- vista de conjunto (NAO imprimir, e so para conferir)
//
//  Referencia da montagem: eixo do tubo = eixo X, na altura z = 0.
//  z = -46 e o topo da borda do aquario.
//
//  openscad -o ../docs/img/montagem.png --imgsize=1600,1200 \
//           --camera=0,0,0,62,0,28,520 assembly.scad
// =====================================================================
include <common.scad>
use <barrel.scad>
use <auger.scad>
use <hopper.scad>
use <lid.scad>
use <chute.scad>
use <bracket.scad>
use <saddle_cap.scad>
use <box_base.scad>
use <box_lid.scad>
use <cam_pod.scad>

SHOW_HOPPER = true;
SHOW_BOX    = true;
SHOW_TANK   = true;
CUTAWAY     = false;   // true = corta tudo em y>0 para ver o miolo
EXPLODE     = 0;       // 0 = montado, 1 = vista explodida

bracket_x = 11;        // inicio do berco ao longo do tubo
bracket_w = 38;
cap_x     = 40;        // tampa do berco (janela livre entre colar e calha)
mast_h_a  = 46;        // igual ao mast_h de bracket.scad
tube_y_a  = 52;        // igual ao tube_y de bracket.scad
mast_cx   = bracket_x + bracket_w/2;

module aquafeeder() {
    e = EXPLODE;
    // ---- mecanismo -------------------------------------------------
    color("#4a6fa5") translate([0,  e*26, 0]) barrel_half("A");
    color("#3d5c8a") translate([0, -e*26, 0]) barrel_half("B");
    color("#d8a13a") translate([0.6 - e*46, 0, 0]) rotate([0, 90, 0]) auger();
    color("#8fbf6a") translate([outlet_x0 + outlet_len/2, 0, -41.4 - e*34]) chute();

    // ---- funil -----------------------------------------------------
    if (SHOW_HOPPER) {
        color("#cfd6dd", 0.55) translate([collar_cx, 0, lug_top + e*44]) hopper();
        color("#9aa5b1") translate([collar_cx, 0, lug_top + 101.2 + 2.6 + e*66])
            rotate([180, 0, 0]) lid();
    }

    // ---- estrutura -------------------------------------------------
    color("#5a5f66") translate([bracket_x, -tube_y_a, -mast_h_a])
        rotate([90, 0, 90]) bracket();
    color("#6e747c") translate([cap_x, 0, EXPLODE*30]) rotate([90, 0, 90]) saddle_cap();

    // ---- eletronica ------------------------------------------------
    if (SHOW_BOX) {
        translate([mast_cx, -56, -16.4]) rotate([0, 90, 0]) rotate([90, 0, 0]) {
            color("#2e3238") box_base();
            color("#454b53") translate([0, 0, box_iz + box_wall + 2.6])
                rotate([180, 0, 0]) box_lid();
        }
        // pe do pod assenta nas 2 bossas do topo da caixa (x = mast_cx +-15)
        color("#b23a3a") translate([mast_cx, -64.4, 42]) cam_pod();
    }

    // ---- contexto: vidro do aquario e linha d'agua ------------------
    // borda do aquario: y de -77 a -53. Agua: y > -53, topo em z = -72.
    if (SHOW_TANK) {
        %translate([-40, -58, -130]) cube([150, 5, 84]);       // vidro
        %translate([-40, -53, -72]) cube([150, 80, 1]);        // superficie da agua
    }
}

if (CUTAWAY)
    difference() { aquafeeder(); translate([-100, 0, -200]) cube([400, 200, 400]); }
else
    aquafeeder();
