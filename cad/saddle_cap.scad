// =====================================================================
//  AquaFeeder -- tampa do berco (fecha o tubo contra a bracadeira)
//  Imprimir EM PE como sai daqui (perfil constante, zero suporte).
//  4x M3 x 16mm roscam direto na bracadeira.
// =====================================================================
include <common.scad>

brk_w       = 9;      // so cabe a janela livre do tubo (x 40..49)
cradle_wall = 4.0;
ear_u       = 6.6;
cr_out      = barrel_od/2 + cradle_wall;
bolt_z      = [brk_w/2];

module cap_profile() {
    difference() {
        union() {
            circle(r = cr_out);
            translate([-cr_out - ear_u, 0]) square([2*(cr_out + ear_u), 7]);
        }
        circle(r = barrel_od/2 + 0.3 + fit/2);
        translate([-40, -40]) square([80, 40]);   // remove tudo abaixo do eixo
    }
}

module saddle_cap() {
    difference() {
        linear_extrude(brk_w) cap_profile();
        for (z = bolt_z, s = [-1, 1])
            translate([s * (cr_out + ear_u/2), 8, z])
                rotate([90, 0, 0]) cylinder(d = m3_free + fit, h = 11);
    }
}

saddle_cap();
