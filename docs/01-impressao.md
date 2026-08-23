# 1. Impressão (Creality K1 Max)

## Material

**PETG.** Ração e água = umidade e respingo; PETG não empena com calor, aguenta
melhor umidade e é mais tenaz que PLA nas roscas dos parafusos.
PLA funciona, mas evite deixar exposto ao sol e não conte com ele por anos.

| Parâmetro | PETG na K1 Max |
|---|---|
| Bico / camada | 0.4 mm / **0.2 mm** |
| Bico | 240–250 °C |
| Mesa | 80 °C |
| Velocidade | 60–120 mm/s (não force PETG a 300 mm/s) |
| Ventilação | 40–60 % |
| Paredes | **3** (4 no tubo) |
| Preenchimento | 20 % (30 % na rosca) |
| Suporte | **NENHUMA peça precisa de suporte** |
| Retração / “combing” | ligue “evitar cruzar contornos” para reduzir fio de PETG |

Todos os STLs já saem na orientação correta de impressão. **Não gire as peças
no fatiador** — a orientação foi escolhida para eliminar suporte e deixar as
camadas na direção certa dos esforços.

## Imprima PRIMEIRO a peça de teste

```
stl/test_fit.stl     ~10 min, 9 g
```

Ela tem quatro gabaritos:

1. **Furos do motor 28BYJ-48** — parafuse o motor de verdade nela, com M3 × 10.
   Os furos têm 2,6 mm (o M3 rosqueia direto no plástico, sem porca), 35 mm entre
   centros, e há um rebaixo para o ressalto do eixo. Se o seu motor for um clone
   com outra medida, corrija `m_hole_pitch` / `m_boss_d` em `cad/common.scad`.
2. **Encaixe do eixo achatado** — enfie o eixo do motor. Deve entrar com
   pressão firme dos dedos, sem folga de giro.
3. **Socket do funil** + 4. **amostra do funil** — devem encaixar com atrito,
   sem forçar.

Se estiver **apertado demais**: abra `cad/common.scad`, mude `fit = 0.10` (ou `0.15`),
rode `cad/build.sh` e reimprima só a peça de teste.
Se estiver **frouxo**: use `fit = -0.10`.
Só depois disso imprima o resto — economiza umas 15 horas de retrabalho.

## Lista de peças

| Arquivo | Qtd | ~Massa | Observação |
|---|---|---|---|
| `auger.stl` | 1 | 3 g | rosca; imprime em pé, cubo na mesa |
| `barrel_a.stl` | 1 | 10 g | metade do tubo — **furos roscados M3** |
| `barrel_b.stl` | 1 | 10 g | metade do tubo — furos passantes + rebaixo da cabeça |
| `hopper.stl` | 1 | 49 g | funil ~300 ml |
| `lid.stl` | 1 | 26 g | tampa do funil |
| `chute.stl` | 1 | 6 g | calha de saída |
| `chute_com_sensor.stl` | (alt.) | 6 g | mesma calha, com 2 furos de 3,4 mm para o sensor óptico — imprima **esta** se for usar sensor de grãos |
| `bracket.stl` | 1 | 66 g | braçadeira + mastro + berço (peça grande, 40 mm em pé) |
| `saddle_cap.stl` | 1 | 2 g | tampa do berço |
| `knob.stl` | 1 | 6 g | manípulo do parafuso de aperto |
| `box_base.stl` | 1 | 31 g | caixa da eletrônica |
| `box_lid.stl` | 1 | 14 g | tampa da caixa |
| `cam_pod.stl` | 1 | 15 g | suporte da câmera (já com o giro de 45° que desvia do funil) |

Total ≈ **240 g**. Cabe tudo em duas mesas na K1 Max (300×300).
Sugestão: mesa 1 = `bracket` + `hopper` + `lid`; mesa 2 = o resto.

## Detalhes que importam

**`auger.stl` (a rosca)** — imprime em pé. A hélice é uma escada helicoidal:
cada camada apoia na anterior, então sai sem suporte. Se a borda da hélice
ficar “peluda”, baixe a velocidade para 40 mm/s só nessa peça. Depois de
imprimir, passe o dedo na hélice para tirar rebarba — rebarba é a causa nº 1
de rosca travando no tubo.

**`barrel_a` / `barrel_b`** — as duas metades do tubo. Saem com a face plana na
mesa, canal para cima. A metade **A** tem furos de 2,55 mm (o parafuso M3 rosca
direto no plástico); a **B** tem furos passantes de 3,3 mm com rebaixo para a
cabeça. Não troque.

**`bracket.stl`** — é um perfil constante extrudado 40 mm: imprime em pé, sem
nenhum overhang. Parece muito material (66 g) porque é a peça que segura ~400 g
pendurados na borda do aquário.

**`hopper.stl`** — parede de 2,2 mm com 3 perímetros fica translúcido, o que é
ótimo: você vê o nível da ração. As paredes têm 72° de inclinação para a ração
escorrer sozinha.

Se quiser um funil maior/menor, mude `hop_taper_h` e `hop_top` em
`cad/common.scad` (o socket de baixo precisa continuar com 25,6 mm).
