// =====================================================================
//  AquaFeeder -- bracadeira da borda do aquario + mastro + berco do tubo
//
//  A peca e um PERFIL CONSTANTE extrudado 40mm: imprima EM PE, como sai
//  deste arquivo. Zero suporte, zero overhang, maxima resistencia.
//
//  Sistema de coordenadas LOCAL (= orientacao de impressao):
//     x local = eixo Y da montagem  (positivo = para dentro do aquario)
//     y local = eixo Z da montagem  (0 = topo da borda do aquario)
//     z local = eixo X da montagem  (comprimento do tubo)
//
//  AJUSTE PRINCIPAL: mast_h define a altura do eixo do tubo acima da borda.
//  A calha termina ~36mm abaixo do eixo. Com mast_h=46 a saida fica ~10mm
//  ACIMA do nivel da borda. Aumente se seu aquario tem a agua muito alta.
// =====================================================================
include <common.scad>

brk_w      = 38;    // largura (ao longo do tubo) -- berco vai de x=11 a x=49
rim_slot   = 24;    // vao da garra (borda de ate ~22mm + parafuso)
jaw_t      = 5;
web_t      = 12;
mast_t     = 8;
mast_h     = 46;    // topo da borda -> eixo do tubo
tube_y     = 52;    // eixo do tubo, medido da face interna da borda
cradle_wall= 4.0;
arm_h      = 13;

cr_out = barrel_od/2 + cradle_wall;   // 14.4
ear_u  = 6.6;                         // saliencia das orelhas do berco
// A tampa do berco cabe so na janela livre do tubo (x 40..49), entao ha
// UM par de parafusos, no meio dessa janela: x=44.5 -> local z=33.5
bolt_z = [33.5];

// ---------------------------------------------------------------------
module brk_profile() {
  difference() {
    union() {
        // garra interna (lado da agua)
        translate([-1, -web_t - clamp_depth]) square([jaw_t + 1, clamp_depth]);
        // garra externa
        translate([-1 - rim_slot - jaw_t, -web_t - clamp_depth]) square([jaw_t, clamp_depth]);
        // teto da garra
        translate([-1 - rim_slot - jaw_t, -web_t]) square([rim_slot + 2*jaw_t + 1, web_t]);
        // boss da porca M4
        translate([-1 - rim_slot - jaw_t - 6, -36]) square([6.1, 15]);
        // mastro
        translate([-mast_t/2, 0]) square([mast_t, mast_h]);
        // reforco mastro/teto
        polygon([[-mast_t/2, 0], [-mast_t/2, 18], [-22, 0]]);
        // braco
        translate([-mast_t/2, mast_h - arm_h]) square([tube_y + mast_t/2, arm_h]);
        // reforco do braco
        polygon([[mast_t/2, mast_h - arm_h], [mast_t/2, 12], [24, mast_h - arm_h]]);
        // berco + orelhas dos parafusos da tampa
        translate([tube_y, mast_h]) circle(r = cr_out);
        translate([tube_y - cr_out - ear_u, mast_h - 7]) square([2*(cr_out + ear_u), 7]);
    }
    // furo do tubo e corte de tudo que passa do eixo (a tampa fecha por cima)
    translate([tube_y, mast_h]) circle(r = barrel_od/2 + fit/2);
    translate([tube_y - 40, mast_h]) square([80, 40]);
  }
}

module bracket() {
    difference() {
        linear_extrude(brk_w) brk_profile();

        // parafusos M3 que prendem a tampa do berco (roscados na peca)
        for (z = bolt_z, s = [-1, 1])
            translate([tube_y + s*(cr_out + ear_u/2), mast_h + 1, z])
                rotate([90, 0, 0]) cylinder(d = m3_tap, h = 9);

        // parafuso de aperto M4 + alojamento da porca
        translate([-1 - rim_slot - jaw_t - 7, -28, brk_w/2]) rotate([0, 90, 0]) {
            cylinder(d = m4_free + fit, h = 20);
            rotate([0, 0, 30]) nut_trap(m4_nut_af + 0.25, m4_nut_t + 0.2);
        }

        // fixacao da caixa de eletronica no mastro (M4 auto-atarraxante)
        for (v = [10, 26])
            translate([-mast_t/2 - 1, v, brk_w/2]) rotate([0, 90, 0])
                cylinder(d = 3.6, h = mast_t + 2);
    }
}

bracket();
