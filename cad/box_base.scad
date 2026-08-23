// =====================================================================
//  AquaFeeder -- caixa da eletronica (base)
//  Na montagem a caixa fica EM PE, presa no mastro da bracadeira, do lado
//  DE FORA do aquario, descendo pela face externa do vidro (igual filtro
//  hang-on). A extremidade -X e o "topo": e la que fica o USB.
//  Imprimir COMO ESTA (boca para cima). Sem suporte.
//
//  ANTES DE IMPRIMIR: confira b1_hx / b1_hy em common.scad com o
//  paquimetro na SUA placa ESP32-S3. Os pinos sao para parafuso
//  auto-atarraxante M2.5. Se preferir, ignore os pinos e use fita dupla
//  face + as fendas de abracadeira.
// =====================================================================
include <common.scad>

ox = box_ix + 2*box_wall;
oy = box_iy + 2*box_wall;
oz = box_iz + box_wall;

post_r  = 4.2;                 // colunas de canto (tampa)
b1_cx   = -24;                 // centro da placa ESP32-S3 (placa ~62mm)
b2_cx   =  30;                 // centro do driver ULN2003 (placa ~35mm)

module corner_posts() {
    for (sx = [-1, 1], sy = [-1, 1])
        translate([sx * (ox/2 - box_wall - post_r + 0.6),
                   sy * (oy/2 - box_wall - post_r + 0.6), box_wall])
            difference() {
                cylinder(r = post_r, h = box_iz);
                translate([0, 0, box_iz - 9]) cylinder(d = m3_tap, h = 10);
            }
}

module pegs(cx, hx, hy) {
    for (sx = [-1, 1], sy = [-1, 1])
        translate([cx + sx*hx/2, sy*hy/2, box_wall - eps])
            difference() {
                cylinder(d = 5.4, h = post_h);
                translate([0, 0, 1]) cylinder(d = peg_d, h = post_h);
            }
}

// fendas para abracadeira de nylon (plano B de fixacao das placas)
module tie_slots() {
    for (x = [b1_cx - 24, b1_cx + 24, b2_cx - 14, b2_cx + 14], sy = [-1, 1])
        translate([x, sy * 17, box_wall/2]) cube([3.4, 8, box_wall + 4], center = true);
}

module box_base() {
    difference() {
        union() {
            // casca
            difference() {
                rbox(ox, oy, oz, 5);
                translate([0, 0, box_wall]) rbox(box_ix, box_iy, oz, 3.5);
            }
            corner_posts();
            pegs(b1_cx, b1_hx, b1_hy);
            pegs(b2_cx, b2_hx, b2_hy);
            // bossas internas para o braco da camera (parede -X)
            for (sy = [-1, 1])
                translate([-ox/2 + box_wall, sy * 15, box_wall])
                    rbox(8, 9, 12, 2);
        }
        // fixacao no mastro (2x M4, parafusos entram por DENTRO da caixa)
        for (x = [-ox/2 + 16, -ox/2 + 32])
            translate([x, 0, -1]) cylinder(d = m4_free + 0.3, h = box_wall + 2);
        // roscas M3 do braco da camera (parede -X, de fora para dentro)
        for (sy = [-1, 1])
            translate([-ox/2 - 1, sy * 15, box_wall + 6])
                rotate([0, 90, 0]) cylinder(d = m3_tap, h = 10);
        // janela do USB-C (topo)
        translate([-ox/2 - 1, -7, box_wall + 1.5]) cube([box_wall + 2, 14, 8]);
        // saida dos cabos (extremidade de baixo) -- faca a alca de gotejamento!
        for (sy = [-1, 1])
            translate([ox/2 + 1, sy * 12, box_wall + 9])
                rotate([0, -90, 0]) cylinder(d = 7.5, h = box_wall + 2);
        // ventilacao (so na face que fica virada para baixo)
        for (i = [-2 : 2])
            translate([ox/2 - box_wall - 1, i * 7, box_wall + 20])
                cube([box_wall + 3, 2.6, 9], center = true);
        tie_slots();
    }
}

box_base();
