// =====================================================================
//  AquaFeeder -- PECA DE TESTE (imprima ESTA PRIMEIRO -- 10 min, ~6 g)
//
//  Valida os 3 encaixes que fazem o projeto funcionar ou nao:
//    1) furos e ressalto do motor 28BYJ-48  (parafuse o motor de verdade)
//    2) encaixe do eixo achatado na rosca   (encaixe o eixo do motor)
//    3) socket do funil                     (so confere a folga do quadrado)
//
//  Se o motor nao entrar: aumente fit em common.scad (0.1 / 0.15) e refaca.
//  Se ficar folgado: use fit negativo (-0.1).
// =====================================================================
include <common.scad>

// --- 1) gabarito do motor -------------------------------------------
module motor_gauge() {
    if (motor_type == "NEMA17") {
        difference() {
            translate([-n17_plate/2, -n17_plate/2, 0])
                cube([n17_plate, n17_plate, n17_plate_t]);
            translate([0, 0, -1]) cylinder(d = n17_shaft_d + 1.2, h = n17_plate_t + 2);
            translate([0, 0, n17_plate_t - n17_boss_h])
                cylinder(d = n17_boss_d + fit, h = n17_boss_h + 1);
            for (sx = [-1, 1], sy = [-1, 1])
                translate([sx * n17_hole_p/2, sy * n17_hole_p/2, -1])
                    cylinder(d = n17_hole_d + fit, h = n17_plate_t + 2);
        }
    } else {
        difference() {
            translate([-24, -plate_w/2, 0]) cube([48, plate_w, plate_t]);
            translate([0, 0, -1]) cylinder(d = m_shaft_d + 1.0, h = plate_t + 2);
            translate([0, 0, -eps]) cylinder(d = m_boss_d + 0.6, h = m_boss_h);
            for (s = [-1, 1])
                translate([0, s * m_hole_pitch/2, -1])
                    cylinder(d = m_hole_d + fit, h = plate_t + 2);
        }
    }
}

// --- 2) encaixe do eixo ---------------------------------------------
module shaft_gauge() {
    if (motor_type == "NEMA17") {
        difference() {
            cylinder(d = hub_d, h = 16);
            translate([0, 0, -eps])
                cylinder(d = n17_shaft_d + 2*n17_socket_c + fit, h = 14);
            translate([0, 0, 7])  rotate([0, 90, 0]) cylinder(d = m3_tap, h = hub_d);
            translate([0, 0, 12]) rotate([90, 0, 0]) cylinder(d = m3_tap, h = hub_d);
        }
    } else {
        difference() {
            cylinder(d = hub_d, h = socket_depth + 3);
            translate([0, 0, -eps]) dshaft(socket_depth + eps, 0.15 + fit/2);
        }
    }
}

// --- 3) socket do funil (so um pedaco da parede) --------------------
module socket_gauge() {
    difference() {
        rbox(socket_sq + 2*collar_wall, socket_sq + 2*collar_wall, 6, 3);
        translate([0, 0, -eps]) rbox(socket_sq + fit, socket_sq + fit, 7, 2);
    }
}

// --- 4) amostra do spigot do funil (encaixa no item 3) -------------
module spigot_gauge() {
    difference() {
        rbox(hop_bot, hop_bot, 6, 3);
        translate([0, 0, -eps]) rbox(hop_bot - 2*hop_wall, hop_bot - 2*hop_wall, 7, 2);
    }
}

gy = (motor_type == "NEMA17") ? n17_plate/2 : plate_w/2;
motor_gauge();
translate([0, gy + 12, 0]) shaft_gauge();
translate([-44, gy + 26, 0]) socket_gauge();
translate([44, gy + 26, 0]) spigot_gauge();
