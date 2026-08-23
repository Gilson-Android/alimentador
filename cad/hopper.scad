// =====================================================================
//  AquaFeeder -- funil (reservatorio de racao) ~300 ml
//  Imprimir COMO ESTA (spigot para baixo). Sem suporte.
//  PETG, 0.2mm, 3 paredes, 15% -- paredes finas, nao precisa forca.
//  Se a racao empedrar, aumente hop_taper_h (funil mais ingreme).
// =====================================================================
include <common.scad>

// z = 0 e a face INFERIOR do flange (o que apoia no colar do tubo)
spig_z0   = -hop_spig_h;                 // -10
flange_t  = 3.2;
body_z0   = flange_t;                    // 3.2
body_z1   = body_z0 + hop_taper_h;       // 98.2
rim_z1    = body_z1 + hop_rim;           // 101.2

flange_x  = 34;
flange_y  = 52;                          // cobre as orelhas do colar (y = +-19.3)

in_bot    = hop_bot - 2*hop_wall;        // 21.2
in_top    = hop_top - 2*hop_wall;        // 81.6

module hopper() {
    difference() {
        union() {
            // spigot (entra no socket do tubo) + flange de apoio
            translate([0, 0, spig_z0]) rbox(hop_bot, hop_bot, hop_spig_h + flange_t, 3);
            rbox(flange_x, flange_y, flange_t, 3);
            // corpo troncoconico
            translate([0, 0, body_z0])
                linear_extrude(hop_taper_h, scale = hop_top / hop_bot)
                    rrect(hop_bot, hop_bot, 3);
            // aro de reforco no topo
            translate([0, 0, body_z1])
                linear_extrude(hop_rim) rrect(hop_top + 4, hop_top + 4, 8);
        }
        // cavidade reta no spigot/flange
        translate([0, 0, spig_z0 - 1])
            rbox(in_bot, in_bot, hop_spig_h + flange_t + 1, 2);
        // cavidade troncoconica
        translate([0, 0, body_z0 - eps])
            linear_extrude(hop_taper_h + hop_rim + 2*eps, scale = in_top / in_bot)
                rrect(in_bot, in_bot, 2);
        // furos M3 -> orelhas do colar (parafuso cabeca panela, sem rebaixo)
        for (s = [-1, 1])
            translate([0, s * lug_y, -1])
                cylinder(d = m3_free + fit, h = flange_t + 2);
    }
}

hopper();
