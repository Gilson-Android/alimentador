// =====================================================================
//  AquaFeeder -- rosca sem-fim (auger)
//  Imprimir EM PE (como esta), cubo para baixo. Sem suporte.
//  0.2 mm de camada / 3 paredes / 30% preenchimento / PETG
// =====================================================================
include <common.scad>

// helice: um setor de coroa circular extrudado com twist gera o helicoide.
// angulo do setor = 360 * espessura_da_aleta / passo
module auger_flight(len) {
    ang   = 360 * flight_t / auger_pitch;
    twist = -360 * len / auger_pitch;
    // slices menor = render mais rapido; 0.6mm por fatia ja fica liso
    linear_extrude(height = len, twist = twist, slices = ceil(len / 0.6), convexity = 8, $fn = 40)
        union() {
            circle(d = auger_core);
            intersection() {
                difference() {
                    circle(d = auger_od);
                    circle(d = auger_core - 1.5);
                }
                pie(auger_od, ang);
            }
        }
}

// furo do parafuso de fixacao do eixo + rebaixo p/ a cabeca na boca externa.
// Desenhado ao longo de +Z: a boca fica na superficie do cubo (raio hub_d/2).
module shaft_screw() {
    cylinder(d = m3_tap, h = hub_d);                          // furo rosqueado
    translate([0, 0, hub_d/2 - n17_screw_head_h])
        cylinder(d = n17_screw_head_d, h = hub_d);            // rebaixo da cabeca
}

module auger() {
    total = hub_len + auger_len + stub_len;
    difference() {
        union() {
            cylinder(d = hub_d, h = hub_len);                          // mancal dianteiro
            cylinder(d = auger_core, h = hub_len + auger_len);         // nucleo
            translate([0, 0, hub_len]) auger_flight(auger_len);        // helice
            translate([0, 0, hub_len + auger_len])                     // mancal traseiro
                cylinder(d = stub_d, h = stub_len);
        }
        // encaixe do eixo do motor
        if (motor_type == "NEMA17") {
            // Encaixe em "D": acompanha o achatado do eixo com folga de deslize
            // (ver n17_socket_c). A face plana pega o torque -- assim a rosca
            // encaixa firme e nao depende do parafuso para nao patinar.
            translate([0, 0, -eps])
                n17_dsocket(n17_socket_d + eps, n17_socket_c + fit/2);
            // DOIS parafusos M3 radiais a 90 graus: o em X cai sobre o achatado
            // (trava axial + reforca o aperto); o em Y e opcional. O torque quem
            // segura e o achatado do encaixe, nao os parafusos.
            translate([0, 0, 7])  rotate([0, 90, 0]) shaft_screw();
            translate([0, 0, 12]) rotate([90, 0, 0]) shaft_screw();
        } else {
            translate([0, 0, -eps]) dshaft(socket_depth + eps, 0.15 + fit/2);
        }

        // chanfro na base (evita pe de elefante travando no tubo)
        difference() {
            translate([0, 0, -eps]) cylinder(d = hub_d + 4, h = 0.9);
            cylinder(d1 = hub_d - 1.8, d2 = hub_d + 0.9, h = 1.0);
        }
    }
    // marca visual do sentido de giro (nao funcional)
}

auger();
