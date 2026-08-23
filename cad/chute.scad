// =====================================================================
//  AquaFeeder -- calha de saida (encaixa por pressao no boss do tubo)
//  Joga a racao para frente e para longe do tubo (menos vapor subindo).
//  Imprimir COMO ESTA. Sem suporte. 3 paredes.
// =====================================================================
include <common.scad>

// Sensor de graos (opcional): abre dois furos de 3.4mm frente/tras para
// encaixar um LED IR de 3mm de um lado e um fototransistor do outro. O feixe
// atravessa o duto e cada grao que passa gera um pulso.
// Deixe false se nao vai usar sensor -- furo aberto deixa escapar racao.
sensor_holes = false;

socket_d = 5.6;                       // profundidade do encaixe
h_duct   = 26;                        // altura do duto
off_y    = 10;                        // deslocamento lateral (inclinacao)
sw       = 2.6;                       // parede do socket

sk_x = boss_x + 0.6 + 2*sw;           // 23.8
sk_y = boss_y + 0.6 + 2*sw;           // 26.8

module chute() {
    difference() {
        union() {
            translate([0, 0, h_duct]) rbox(sk_x, sk_y, socket_d, 3);
            hull() {
                translate([0, 0, h_duct - 0.01]) rbox(sk_x, sk_y, 0.01, 3);
                translate([0, off_y, 0])         rbox(22, 24, 0.01, 3);
            }
        }
        // cavidade do encaixe
        translate([0, 0, h_duct - eps])
            rbox(boss_x + 0.5 + fit, boss_y + 0.5 + fit, socket_d + 1, 2);
        // covinhas que travam nas meias-esferas do tubo
        for (s = [-1, 1])
            translate([0, s * (boss_y + 0.5)/2, h_duct + 2.6]) sphere(r = 2.0);
        // duto interno
        hull() {
            translate([0, 0, h_duct - 0.01]) rbox(boss_x + 0.5, boss_y + 0.5, 0.02, 2);
            translate([0, off_y, -1])        rbox(15.5, 17, 1.02, 2);
        }
        // furos do sensor optico (atravessam o duto)
        if (sensor_holes)
            translate([-16, off_y/2, 13]) rotate([0, 90, 0]) cylinder(d = 3.4, h = 32);
    }
}

chute();
