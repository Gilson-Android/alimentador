// =====================================================================
//  AquaFeeder ESP32-S3  --  parametros comuns a todas as pecas
//  Unidades: milimetros. Impressora alvo: Creality K1 Max, bico 0.4mm
// =====================================================================
$fa = 4;
$fs = 0.4;
eps = 0.02;

// Folga extra aplicada a TODOS os encaixes. Se sua K1 Max estiver saindo
// justa demais, use fit = 0.1 ou 0.15 e reimprima so as pecas do encaixe.
fit = 0.0;

// =====================================================================
//  ESCOLHA DO MOTOR
//    "28BYJ"  = motor de passo 5V com redutor + driver ULN2003 (padrao)
//    "NEMA17" = NEMA 17 de impressora 3D + driver A4988/DRV8825/TMC + 12V
//
//  Muda apenas DUAS pecas: barrel_a/barrel_b (fixacao do motor) e auger
//  (encaixe no eixo). Todo o resto e igual nos dois casos.
//    openscad -o barrel_a.stl -D 'PART="A"' -D 'motor_type="NEMA17"' barrel.scad
// =====================================================================
motor_type = "28BYJ";

// ---------------------------------------------------------------------
//  Rosca sem-fim (auger)
// ---------------------------------------------------------------------
auger_od     = 14.0;   // diametro externo da helice
auger_core   = 6.0;    // diametro do nucleo
auger_pitch  = 9.0;    // passo da helice (avanco por volta)
flight_t     = 1.6;    // espessura vertical da aleta
hub_d        = 14.6;   // cubo dianteiro (funciona como mancal no tubo)
// O cubo tem que ser fundo o suficiente para engolir o eixo do motor.
// O NEMA 17 tem eixo liso de ~20-25mm, por isso o cubo dele e maior --
// e por isso a janela de entrada comeca mais para tras (ver abaixo).
hub_len      = (motor_type == "NEMA17") ? 22.0 : 11.0;
stub_d       = 5.8;    // ponta traseira (segundo mancal)
stub_len     = 3.2;
// Folga radial helice <-> tubo. Cada metade do tubo imprime com a face plana
// na mesa, entao o "teto" do furo (que na montagem fica na LATERAL, na altura
// do eixo) e um arco e cede uns 0.2-0.4mm na impressao. A folga de 0.6 existe
// para engolir essa barriga: com 0.4 a helice raspava.
radial_clear = 0.60;

// Volume teorico por volta:
//   area do canal = pi/4*(od^2 - core^2) = 125.7 mm2
//   volume/volta  = area * (passo - espessura_aleta) ~= 0.93 ml
//   1 porcao padrao no firmware = 1/4 de volta ~= 0.23 ml

// ---------------------------------------------------------------------
//  Funil (hopper) ~ 300 ml
// ---------------------------------------------------------------------
hop_wall    = 2.2;
hop_bot     = 25.6;    // spigot: entra no socket de 26.0
hop_top     = 86.0;
hop_taper_h = 95.0;
hop_insert  = 10.0;    // trecho reto que entra no socket do colar
hop_cham    = 9.0;     // chanfro (40 graus) entre o spigot e o flange
hop_spig_h  = hop_insert + hop_cham;
hop_rim     = 3.0;     // aba superior onde a tampa agarra
hop_rim_ch  = 3.5;     // chanfro embaixo do aro (inclusive nos cantos!)
hop_flg_x   = 34.0;    // flange
hop_flg_y   = 41.0;
hop_flg_t   = 3.2;

// ---------------------------------------------------------------------
//  Tubo transportador (barrel) -- eixo ao longo de X, de x=0 a x=barrel_len
// ---------------------------------------------------------------------
bore_d      = auger_od + 2*radial_clear;    // 14.8
barrel_wall = 3.0;
barrel_od   = bore_d + 2*barrel_wall;       // 20.8
end_wall    = 4.0;                          // parede do fundo (mancal traseiro)

// A janela de entrada precisa ficar TODA sobre a helice (sobre o cubo liso
// nao passa racao), por isso tudo e derivado de hub_len.
inlet_x0    = hub_len + 2;   inlet_len  = 22;   inlet_w  = 13;   // entrada (topo)
outlet_x0   = inlet_x0 + inlet_len + 18;
outlet_len  = 14;            outlet_w   = 13;                    // saida (fundo)

bore_end    = outlet_x0 + outlet_len + 1;   // onde o furo interno termina
barrel_len  = bore_end + end_wall;
auger_len   = bore_end - 1 - hub_len;       // helice: do fim do cubo ate o fundo

collar_cx   = inlet_x0 + inlet_len/2;   // centro do colar do funil
socket_sq   = 26.0;                     // lado INTERNO do socket do funil
collar_wall = 2.8;
collar_z0   = 4;                        // base do colar (dentro do tubo)
collar_z1   = 21;                       // topo do colar = apoio do flange do funil
// Orelhas do colar: recebem os 2 parafusos do flange do funil. Elas sobem
// hop_cham acima do topo do colar porque o flange do funil ficou mais alto --
// e o flange subiu para caber um chanfro de 40 graus embaixo dele, senao o
// funil precisaria de suporte na impressao.
lug_y       = 16.0;                     // centro das orelhas em Y
lug_top     = collar_z1 + hop_cham;     // face de apoio do flange
lug_t       = lug_top - 15;             // espessura da orelha

boss_x      = 18;    // boss da calha (encaixe), comprimento em X
boss_y      = 21;    // largura em Y
boss_h      = 5;     // quanto desce abaixo do tubo
boss_rim    = 0.8;   // ressalto do engate rapido da calha

ear_w       = 9;     // orelhas de parafuso (unem as duas metades)
ear_out     = 9;
ear_t       = 5;     // espessura de CADA metade da orelha

// Zonas ocupadas do tubo:
//   topo:  orelha 0..9 | colar collar_cx +- 15.8 | orelha (barrel_len-9)..fim
//   fundo: orelha 0..9 | boss da calha outlet_cx +- 9
// A bracadeira abraca o tubo POR BAIXO, entre a orelha da frente e o boss;
// a tampa do berco fecha POR CIMA, entre o colar e o boss.
//   28BYJ  -> berco em x 11..49, tampa em x 40..49
//   NEMA17 -> berco em x 24..62, tampa em x 53..62  (a peca e a mesma,
//             so muda onde voce posiciona a bracadeira no tubo)

// O tubo e partido no plano y=0 (metades A e B). A juncao e topo a topo,
// apertada por: 3 parafusos das orelhas + 2 do flange do funil + 2 do motor
// + o encaixe da calha. Nao ha macho/femea -- isso permite imprimir as duas
// metades com a face plana na mesa, sem nenhum suporte.


// ---------------------------------------------------------------------
//  Motor 28BYJ-48  (CONFIRA COM PAQUIMETRO -- ha clones fora de padrao)
//  Imprima test_fit.scad antes de imprimir o tubo.
// ---------------------------------------------------------------------
m_shaft_d    = 5.0;    // eixo: 5mm com dois lados achatados
m_shaft_flat = 3.0;    // distancia entre as faces achatadas
m_boss_d     = 9.6;    // ressalto em volta do eixo
m_boss_h     = 1.8;
m_hole_pitch = 35.0;   // centro a centro dos furos das abas
m_hole_d     = 2.6;    // M3 rosqueia direto no plastico (sem porca)
m_body_d     = 28.4;
m_offset     = 8.0;    // eixo -> centro do corpo (corpo fica ABAIXO do eixo)
socket_depth = 7.5;    // profundidade do encaixe do eixo na rosca

plate_t      = 3.2;    // prato de fixacao do motor
plate_w      = 46;     // em Y
plate_h      = 42;     // em Z
plate_dz     = -8;     // centro do prato em Z (desce para cobrir o motor)

// ---------------------------------------------------------------------
//  Motor NEMA 17 (o de impressora 3D)
//  Eixo liso de 5mm e LONGO (20-25mm). Em vez de pedir para voce cortar o
//  eixo, o cubo da rosca engole ele inteiro (n17_socket_d) -- e por isso o
//  cubo do NEMA e maior e a janela de entrada comeca mais para tras.
//  Fixacao: prato de 52x52 com os 4 furos M3 em quadrado de 31mm.
// ---------------------------------------------------------------------
n17_hole_p   = 31.0;   // furos M3 em quadrado de 31mm
n17_hole_d   = 3.4;
n17_boss_d   = 22.4;   // ressalto central do motor
n17_boss_h   = 2.6;
n17_shaft_d  = 5.0;    // diametro do eixo
n17_flat     = 4.6;    // cota do achatado (do plano ate o lado oposto)
// Folga do encaixe. Furo vertical em FDM sai 0.1-0.3mm menor que o nominal:
// com 0.15 o eixo de aco nao entrava sem broca. Com 0.35 desliza e o parafuso
// de fixacao tira a folga -- quem centra a rosca sao os mancais dela, nao o eixo.
n17_socket_c = 0.35;
n17_plate    = 52;     // lado do prato
n17_plate_t  = 4.5;
n17_socket_d = 20;     // encaixe fundo: engole eixo de ate ~24mm sem cortar

// ---------------------------------------------------------------------
//  Bracadeira do aquario
// ---------------------------------------------------------------------
rim_max     = 22;    // espessura maxima da borda/moldura do aquario
clamp_wall  = 5;
clamp_depth = 34;    // quanto a garra avanca sobre a borda
arm_reach   = 46;    // do centro da garra ate o centro do tubo
arm_rise    = 30;    // altura do tubo acima da borda
arm_t       = 8;     // espessura da alma do braco

// ---------------------------------------------------------------------
//  Caixa da eletronica
// ---------------------------------------------------------------------
box_ix      = 112;   // interno X
box_iy      = 48;    // interno Y
box_iz      = 27;    // interno Z
box_wall    = 2.4;
box_lip     = 4;

// Placa 1 = ESP32-S3 CAM (padrao: Freenove ESP32-S3-WROOM ~ 62 x 26 mm)
b1_hx       = 55;    // distancia entre furos em X  <-- MEDIR
b1_hy       = 20;    // distancia entre furos em Y  <-- MEDIR
// Placa 2 = driver ULN2003 (35 x 32 mm, furos 30 x 27)
b2_hx       = 30;
b2_hy       = 27;
post_h      = 5;
peg_d       = 2.3;   // pino para parafuso auto-atarraxante M2.5

// Modulo de camera OV2640 (PCB ~ 24.5 x 24.5, 2 furos M2)
cam_pcb     = 24.5;
cam_hole_p  = 21.0;
cam_lens_d  = 9.2;

// ---------------------------------------------------------------------
//  Parafusos
// ---------------------------------------------------------------------
m3_free = 3.3;  m3_tap = 2.55; m3_head = 6.2; m3_head_h = 2.2;
m4_free = 4.4;  m4_nut_af = 7.3; m4_nut_t = 3.4;

// =====================================================================
//  Utilitarios
// =====================================================================

// cilindro com eixo em X, de x0 a x1
module xcyl(d, x0, x1) {
    translate([x0, 0, 0]) rotate([0, 90, 0]) cylinder(d = d, h = x1 - x0);
}

// retangulo 2D com cantos arredondados, centrado
module rrect(x, y, r) {
    offset(r = r) square([x - 2*r, y - 2*r], center = true);
}

// caixa 3D com cantos arredondados no plano XY
module rbox(x, y, z, r) {
    linear_extrude(z) rrect(x, y, r);
}

// eixo do 28BYJ-48 (duplo achatamento), extra = folga radial
module dshaft(len, extra = 0.15) {
    intersection() {
        cylinder(d = m_shaft_d + 2*extra, h = len);
        translate([0, 0, len/2])
            cube([m_shaft_d + 2*extra + 2, m_shaft_flat + 2*extra, len], center = true);
    }
}

// setor de circulo (pizza) com angulo ang, raio r
module pie(r, ang) {
    n = max(8, ceil(ang / 4));
    polygon(concat([[0, 0]], [for (i = [0 : n]) let (a = ang * i / n) [r*cos(a), r*sin(a)]]));
}

// furo para porca sextavada (nut trap)
module nut_trap(af, t) {
    cylinder(d = af / cos(30), h = t, $fn = 6);
}
