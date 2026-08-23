// =====================================================================
//  AquaFeeder -- tubo transportador, partido ao meio no plano y=0
//
//  openscad -o barrel_a.stl -D 'PART="A"' barrel.scad
//  openscad -o barrel_b.stl -D 'PART="B"' barrel.scad
//
//  A = metade com roscas M3 (furo 2.55)   B = metade com furos passantes
//  Imprimir com a face plana na mesa (a orientacao ja sai pronta). SEM SUPORTE.
//  4 paredes / 25% / PETG.
// =====================================================================
include <common.scad>

PART  = "A";
PRINT = true;

ear_z0   = 5;
ear_z1   = barrel_od/2 + ear_out;
ear_screw_z = 15.5;
outlet_cx   = outlet_x0 + outlet_len/2;
boss_z0     = -barrel_od/2 - boss_h;

// posicao das orelhas: [x0, lado]  (+1 = topo, -1 = fundo)
ears = [[0, 1], [0, -1], [barrel_len - ear_w, 1]];

// ---------------------------------------------------------------------
module ear(x0, s) {
    rotate([90, 0, 0])
        linear_extrude(height = 2*ear_t, center = true)
            translate([x0 + ear_w/2, s * (ear_z0 + ear_z1)/2])
                rrect(ear_w, ear_z1 - ear_z0, 2.5);
}

module ear_screw(x0, s, part) {
    translate([x0 + ear_w/2, 0, s * ear_screw_z]) rotate([-90, 0, 0]) {
        if (part == "A") {
            translate([0, 0, -0.2]) cylinder(d = m3_tap, h = ear_t + 0.4);
        } else {
            translate([0, 0, -ear_t - 0.2]) cylinder(d = m3_free + fit, h = ear_t + 0.4);
            translate([0, 0, -ear_t - 0.2]) cylinder(d = m3_head, h = m3_head_h + 0.2);
        }
    }
}

// ---------------------------------------------------------------------
// Fixacao do motor -- muda conforme motor_type (ver common.scad)
module motor_plate() {
    if (motor_type == "NEMA17") {
        translate([-n17_plate_t, 0, 0]) rotate([0, 90, 0])
            linear_extrude(n17_plate_t) rrect(n17_plate, n17_plate, 5);
    } else {
        translate([-plate_t, 0, plate_dz]) rotate([0, 90, 0])
            linear_extrude(plate_t) rrect(plate_h, plate_w, 4);
    }
}

module motor_cuts() {
    if (motor_type == "NEMA17") {
        xcyl(n17_shaft_d + 1.2, -n17_plate_t - 1, 1.0);                    // eixo
        xcyl(n17_boss_d + fit, -n17_plate_t - eps, -n17_plate_t + n17_boss_h);
        // 4 furos M3: o parafuso entra por dentro e rosca no proprio motor
        for (sy = [-1, 1], sz = [-1, 1])
            translate([-n17_plate_t - 1, sy * n17_hole_p/2, sz * n17_hole_p/2])
                rotate([0, 90, 0]) cylinder(d = n17_hole_d + fit, h = n17_plate_t + 2);
    } else {
        xcyl(m_shaft_d + 1.0, -plate_t - 1, 1.0);
        xcyl(m_boss_d + 0.6, -plate_t - eps, -plate_t + m_boss_h);
        for (s = [-1, 1])
            translate([-plate_t - 1, s * m_hole_pitch/2, 0])
                rotate([0, 90, 0]) cylinder(d = m_hole_d + fit, h = plate_t + 2);
    }
}

module collar() {
    // colar (socket do funil)
    translate([collar_cx, 0, collar_z0])
        linear_extrude(collar_z1 - collar_z0)
            rrect(socket_sq + 2*collar_wall, socket_sq + 2*collar_wall, 3);
    // orelhas laterais que recebem o flange do funil
    for (s = [-1, 1])
        translate([collar_cx, s * lug_y, collar_z1 - lug_t])
            rbox(14, 11, lug_t, 2);
}

module outlet_boss() {
    translate([outlet_cx, 0, boss_z0])
        linear_extrude(boss_h + 6) rrect(boss_x, boss_y, 2);
    // meias-esferas de retencao da calha (uma em cada metade)
    for (s = [-1, 1])
        translate([outlet_cx, s * (boss_y/2 - 1.1), boss_z0 + 2.6]) sphere(r = 1.8);
}

// ---------------------------------------------------------------------
module barrel_solid() {
    union() {
        xcyl(barrel_od, 0, barrel_len);
        motor_plate();
        collar();
        outlet_boss();
        for (e = ears) ear(e[0], e[1]);
    }
}

module barrel_cuts(part) {
    // furo interno + bolsa do mancal traseiro
    xcyl(bore_d + fit, 0, bore_end);
    xcyl(stub_d + 0.8 + fit, bore_end, bore_end + stub_len + 0.4);

    // eixo, ressalto e furos do motor escolhido
    motor_cuts();

    // socket do funil + funil de transicao ate a janela de entrada
    translate([collar_cx, 0, 8])
        linear_extrude(collar_z1 - 8 + 1)
            rrect(socket_sq + fit, socket_sq + fit, 2);
    hull() {
        translate([collar_cx, 0, 8 - 0.1])
            linear_extrude(0.1) rrect(socket_sq + fit, socket_sq + fit, 2);
        translate([collar_cx, 0, 1.5])
            linear_extrude(0.1) rrect(inlet_len, inlet_w, 2);
    }

    // saida (fundo)
    translate([outlet_cx, 0, -12])
        cube([outlet_len, outlet_w, 14], center = true);

    // roscas M3 das orelhas do colar (flange do funil entra por cima)
    for (s = [-1, 1])
        translate([collar_cx, s * lug_y, collar_z1 + 0.2])
            rotate([180, 0, 0]) cylinder(d = m3_tap, h = lug_t - 0.6);

    // parafusos que unem as metades
    for (e = ears) ear_screw(e[0], e[1], part);
}

module half_space(a) {
    translate([-80, a ? 0 : -200, -120]) cube([320, 200, 240]);
}

module barrel_half(part) {
    difference() {
        intersection() {
            barrel_solid();
            half_space(part == "A");
        }
        barrel_cuts(part);
    }
}

// ---------------------------------------------------------------------
if (PRINT)
    rotate([(PART == "A") ? 90 : -90, 0, 0]) barrel_half(PART);
else
    barrel_half(PART);
