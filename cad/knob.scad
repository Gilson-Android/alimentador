// =====================================================================
//  AquaFeeder -- manipulo do parafuso de aperto da bracadeira
//  Encaixe por pressao na cabeca de um parafuso M4 (sextavado 7mm).
//  Se o seu parafuso for de cabeca cilindrica com fenda, use uma chave.
// =====================================================================
include <common.scad>

module knob() {
    difference() {
        union() {
            cylinder(d = 26, h = 11);
            for (a = [0 : 45 : 359])
                rotate([0, 0, a]) translate([13, 0, 0]) cylinder(d = 8, h = 11);
        }
        // passagem da rosca
        translate([0, 0, 3.4 - eps]) cylinder(d = m4_free + 0.3, h = 12);
        // alojamento da cabeca sextavada M4
        translate([0, 0, -eps]) nut_trap(7.0 + fit, 3.4);
        // rebaixo estetico
        translate([0, 0, 9.5]) cylinder(d = 18, h = 2);
    }
}

knob();
