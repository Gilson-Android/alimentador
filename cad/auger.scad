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
            // Encaixe redondo com folga de deslize (ver n17_socket_c).
            translate([0, 0, -eps])
                cylinder(d = n17_shaft_d + 2*n17_socket_c + fit, h = n17_socket_d + eps);
            // DOIS parafusos M3 radiais a 90 graus: use o que cair em cima do
            // achatado do eixo (ou os dois). E o achatado que pega o torque.
            translate([0, 0, 7])  rotate([0, 90, 0]) cylinder(d = m3_tap, h = hub_d);
            translate([0, 0, 12]) rotate([90, 0, 0]) cylinder(d = m3_tap, h = hub_d);
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
