// =====================================================================
//  Teste de interferencia: as pecas ocupam o mesmo espaco?
//
//  Renderiza a INTERSECAO de dois conjuntos de pecas na posicao de
//  montagem. Se sair QUALQUER volume, ha colisao -- as pecas nao encaixam.
//
//    openscad -o /tmp/i.stl -D 'PAR="funil_tubo"' check_interferencia.scad
//
//  Volume esperado: ZERO (o STL sai vazio e o OpenSCAD avisa
//  "Current top level object is empty").
// =====================================================================
include <common.scad>
use <barrel.scad>
use <hopper.scad>
use <chute.scad>
use <auger.scad>
use <bracket.scad>
use <saddle_cap.scad>

PAR = "funil_tubo";

module tubo() {
    barrel_half("A");
    barrel_half("B");
}
module funil() {
    translate([collar_cx, 0, lug_top]) hopper();
}
module rosca() {
    translate([0.6, 0, 0]) rotate([0, 90, 0]) auger();
}
module calha() {
    translate([outlet_x0 + outlet_len/2, 0, -41.4]) chute();
}
// posicao da bracadeira depende do motor (ver zonas livres em common.scad)
brk_x = (motor_type == "NEMA17") ? 24 : 11;
cap_x = (motor_type == "NEMA17") ? 53 : 40;
module suporte() {
    translate([brk_x, -52, -46]) rotate([90, 0, 90]) bracket();
    translate([cap_x, 0, 0]) rotate([90, 0, 90]) saddle_cap();
}

intersection() {
    if      (PAR == "funil_tubo")  funil();
    else if (PAR == "rosca_tubo")  rosca();
    else if (PAR == "calha_tubo")  calha();
    else if (PAR == "suporte_tubo") suporte();
    else if (PAR == "funil_suporte") funil();

    if      (PAR == "funil_tubo")   tubo();
    else if (PAR == "rosca_tubo")   tubo();
    else if (PAR == "calha_tubo")   tubo();
    else if (PAR == "suporte_tubo") tubo();
    else if (PAR == "funil_suporte") suporte();
}
