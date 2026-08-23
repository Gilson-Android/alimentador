// =====================================================================
//  AquaFeeder -- suporte da camera (modulo OV2640)
//  Parafusa em cima da caixa da eletronica (2x M3 x 12, 30mm entre furos).
//
//  DUAS inclinacoes importam:
//    cam_tilt = quanto aponta para baixo
//    cam_yaw  = quanto gira na horizontal PARA DESVIAR DO FUNIL.
//  Com yaw = 0 a camera olha direto para a parede do funil e nao ve nada.
//  Com yaw = 45 ela olha na diagonal, passa ao lado do funil e enquadra a
//  superficie da agua + a saida da calha.
//
//  Imprimir COMO ESTA (pe na mesa). Sem suporte.
// =====================================================================
include <common.scad>

pod_w     = 36;    // largura da placa da camera (= largura do pe)
foot_x    = 36;    // pe: X = direcao dos 2 parafusos
foot_y    = 22;
foot_t    = 6;
post_d    = 18;
post_dy   = -4;    // poste recuado, para nao bater no mastro
post_top  = 40;
plate_t   = 3.4;
plate_l   = 30;
plate_off = 9;     // placa adiantada em relacao ao eixo do poste

cam_tilt  = 45;    // graus para baixo
cam_yaw   = 45;    // graus na horizontal (desvia do funil)

module pod_base() {
    difference() {
        union() {
            // pe
            hull() {
                rbox(foot_x, 4, foot_t, 1.8);
                translate([0, post_dy - foot_y/2 + 2, 0]) rbox(foot_x - 6, 4, foot_t, 1.8);
            }
            // poste + concordancia
            translate([0, post_dy, foot_t - eps]) {
                cylinder(d1 = post_d + 9, d2 = post_d, h = 5);
                cylinder(d = post_d, h = post_top - foot_t);
            }
        }
        // parafusos para as bossas da caixa
        for (s = [-1, 1])
            translate([s * 15, 0, -1]) cylinder(d = m3_free + fit, h = foot_t + 2);
    }
}

// cabeca: origem no topo do poste, +x = direcao do olhar antes das rotacoes
module cam_head() {
    rotate([0, 0, cam_yaw]) rotate([0, cam_tilt, 0]) {
        difference() {
            union() {
                translate([plate_off, 0, 0])
                    cube([plate_t, pod_w, plate_l], center = true);
                // reforco: liga a COSTAS da placa ao poste (nunca invade a
                // frente, senao a PCB da camera nao assenta)
                hull() {
                    translate([plate_off - plate_t/2 - 0.5, 0, 0])
                        cube([1, pod_w, plate_l], center = true);
                    translate([-3, 0, 0]) cube([1, 16, 9], center = true);
                }
            }
            // janela da lente
            translate([plate_off, 0, 0]) rotate([0, 90, 0])
                cylinder(d = cam_lens_d, h = 40, center = true);
            // furos M2 da PCB
            for (s = [-1, 1])
                translate([plate_off, s * cam_hole_p/2, 0]) rotate([0, 90, 0])
                    cylinder(d = 2.4, h = 40, center = true);
        }
    }
}

module cam_pod() {
    union() {
        pod_base();
        translate([0, post_dy, post_top]) cam_head();
    }
}

cam_pod();
