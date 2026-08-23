// =====================================================================
//  AquaFeeder -- funil (reservatorio de racao) ~300 ml
//  Imprimir COMO ESTA (spigot para baixo). SEM SUPORTE.
//  PETG, 0.2mm, 3 paredes, 15% -- paredes finas, nao precisa forca.
//
//  Por que o pescoco e comprido: o flange precisa de um chanfro de 40 graus
//  embaixo dele, senao ele salta no ar e o fatiador pede suporte. O chanfro
//  come 9mm de altura, entao o flange fica 9mm acima do topo do colar -- e as
//  orelhas do colar (no tubo) sobem a mesma coisa para receber os parafusos.
// =====================================================================
include <common.scad>

// z = 0 e a face INFERIOR do flange (o que apoia nas orelhas do colar)
spig_z0   = -hop_spig_h;                 // -19
cham_z0   = -hop_cham;                   // -9  inicio do chanfro
body_z0   = hop_flg_t;                   // 3.2
body_z1   = body_z0 + hop_taper_h;       // 98.2
rim_z0    = body_z1;
rim_z1    = body_z1 + hop_rim;

in_bot    = hop_bot - 2*hop_wall;        // 21.2
in_top    = hop_top - 2*hop_wall;        // 81.6

module hopper() {
    difference() {
        union() {
            // trecho reto que entra no socket do colar
            translate([0, 0, spig_z0]) rbox(hop_bot, hop_bot, hop_insert, 3);
            // chanfro 40 graus: spigot -> flange (e o que dispensa suporte)
            hull() {
                translate([0, 0, cham_z0]) rbox(hop_bot, hop_bot, 0.01, 3);
                translate([0, 0, -0.01]) rbox(hop_flg_x, hop_flg_y, 0.01, 3);
            }
            // flange
            rbox(hop_flg_x, hop_flg_y, hop_flg_t, 3);
            // corpo troncoconico
            translate([0, 0, body_z0])
                linear_extrude(hop_taper_h, scale = hop_top / hop_bot)
                    rrect(hop_bot, hop_bot, 3);
            // aro de reforco no topo. O chanfro comeca EXATAMENTE na superficie
            // do corpo (86mm), senao sobra um degrau virado para baixo e o
            // fatiador pede suporte ali.
            hull() {
                translate([0, 0, rim_z0 - 0.01])
                    linear_extrude(0.01) rrect(hop_top, hop_top, 8);
                translate([0, 0, rim_z0 + hop_rim_ch])
                    linear_extrude(0.01) rrect(hop_top + 4, hop_top + 4, 8);
            }
            translate([0, 0, rim_z0 + hop_rim_ch]) linear_extrude(hop_rim)
                rrect(hop_top + 4, hop_top + 4, 8);
        }
        // cavidade reta no pescoco (spigot + chanfro + flange)
        translate([0, 0, spig_z0 - 1])
            rbox(in_bot, in_bot, hop_spig_h + hop_flg_t + 1, 2);
        // cavidade troncoconica
        translate([0, 0, body_z0 - eps])
            linear_extrude(hop_taper_h + hop_rim_ch + hop_rim + 2*eps,
                           scale = in_top / in_bot)
                rrect(in_bot, in_bot, 2);
        // furos M3 -> orelhas do colar (parafuso cabeca panela, sem rebaixo)
        for (s = [-1, 1])
            translate([0, s * lug_y, -1])
                cylinder(d = m3_free + fit, h = hop_flg_t + 2);
    }
}

hopper();
