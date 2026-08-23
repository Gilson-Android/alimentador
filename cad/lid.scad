// =====================================================================
//  AquaFeeder -- tampa do funil (encaixa por pressao no aro do funil)
//  Imprimir COMO ESTA (topo na mesa, saia para cima). Sem suporte.
//  Dica: jogue um sache de silica gel dentro do funil -- racao umida
//  empedra e entope a rosca.
// =====================================================================
include <common.scad>

band_od    = hop_top + 4;        // 90 -- aro do funil
skirt_in   = band_od + 0.5 + fit;
skirt_wall = 2.2;
skirt_h    = 11;
top_t      = 2.6;
ridge      = 0.7;                // ressalto interno de encaixe
ridge_z    = 4.0;                // logo abaixo do aro do funil (aro = 3mm)

out_sq = skirt_in + 2*skirt_wall;

// cavidade da saia, com uma "cintura" que gera o friso de encaixe
module skirt_cavity() {
    translate([0, 0, top_t])
        rbox(skirt_in, skirt_in, ridge_z - 1.2, 9);
    translate([0, 0, top_t + ridge_z - 1.2]) hull() {
        rbox(skirt_in, skirt_in, 0.01, 9);
        translate([0, 0, 1.2]) rbox(skirt_in - 2*ridge, skirt_in - 2*ridge, 0.01, 9);
    }
    translate([0, 0, top_t + ridge_z]) hull() {
        rbox(skirt_in - 2*ridge, skirt_in - 2*ridge, 0.01, 9);
        translate([0, 0, 1.8]) rbox(skirt_in, skirt_in, 0.01, 9);
    }
    translate([0, 0, top_t + ridge_z + 1.8])
        rbox(skirt_in, skirt_in, skirt_h, 9);
}

module lid() {
    difference() {
        union() {
            rbox(out_sq, out_sq, top_t + skirt_h, 10);
            // abas para tirar a tampa com o dedo
            for (s = [-1, 1])
                translate([s * (out_sq/2 - 2), 0, 0])
                    rbox(16, 28, top_t + 3.0, 3);
        }
        skirt_cavity();
        // respiro (equaliza pressao sem deixar entrar umidade demais)
        for (a = [0, 90, 180, 270])
            rotate([0, 0, a]) translate([20, 0, -1]) cylinder(d = 1.4, h = top_t + 2);
    }
}

lid();
