// =====================================================================
//  AquaFeeder -- tampa da caixa da eletronica
//  4x M3 x 10mm. O labio interno cria uma barreira contra respingo.
//  Imprimir COMO ESTA (labio para cima). Sem suporte.
// =====================================================================
include <common.scad>

ox = box_ix + 2*box_wall;
oy = box_iy + 2*box_wall;
lid_t   = 2.6;
post_r  = 4.2;
hx      = ox/2 - box_wall - post_r + 0.6;
hy      = oy/2 - box_wall - post_r + 0.6;

module box_lid() {
    difference() {
        union() {
            rbox(ox, oy, lid_t, 5);
            // labio que entra na caixa
            translate([0, 0, lid_t - eps])
                rbox(box_ix - 0.5, box_iy - 0.5, box_lip, 3.5);
        }
        // vazio do labio (o labio e so um anel de vedacao)
        translate([0, 0, lid_t])
            rbox(box_ix - 0.5 - 2*2.0, box_iy - 0.5 - 2*2.0, box_lip + 1, 2.5);
        // folga para as colunas de canto da caixa
        for (sx = [-1, 1], sy = [-1, 1])
            translate([sx * hx, sy * hy, lid_t - eps])
                cylinder(r = post_r + 0.4, h = box_lip + 2);
        // parafusos
        for (sx = [-1, 1], sy = [-1, 1])
            translate([sx * hx, sy * hy, -1])
                cylinder(d = m3_free + fit, h = lid_t + box_lip + 2);
    }
}

box_lid();
