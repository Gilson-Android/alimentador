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
stl/motor_28byj/peca_de_teste.stl     ~10 min,  9 g
stl/motor_nema17/peca_de_teste.stl    ~14 min, 12 g
```

Só a do **seu** motor.

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

As quatro peças que dependem do motor ficam em `stl/motor_28byj/` e
`stl/motor_nema17/`. **Imprima só a pasta do seu motor.**

| Arquivo | Qtd | ~Massa | Observação |
|---|---|---|---|
| `motor_*/rosca.stl` | 1 | 3–4 g | a rosca; imprime em pé, cubo na mesa |
| `motor_*/tubo_metade_a.stl` | 1 | 10–13 g | metade do tubo — **furos roscados** M3 (2,55 mm) |
| `motor_*/tubo_metade_b.stl` | 1 | 10–13 g | metade do tubo — furos passantes 3,3 mm com rebaixo |
| `motor_*/peca_de_teste.stl` | 1 | 9–12 g | **imprima esta primeiro** |
| `funil.stl` | 1 | 65 g | reservatório ~300 ml, sai translúcido; 124 mm de altura |
| `tampa_do_funil.stl` | 1 | 26 g | |
| `calha.stl` | 1 | 6 g | calha de saída |
| `calha_com_sensor.stl` | (alt.) | 6 g | mesma calha, com 2 furos de 3,4 mm para o sensor óptico |
| `bracadeira.stl` | 1 | 66 g | braçadeira + mastro + berço; imprime em pé, 38 mm |
| `tampa_do_berco.stl` | 1 | 2 g | fecha o tubo no berço |
| `manipulo.stl` | 1 | 6 g | manípulo do parafuso de aperto |
| `caixa_eletronica.stl` | 1 | 31 g | |
| `tampa_da_caixa.stl` | 1 | 14 g | |
| `suporte_da_camera.stl` | 1 | 15 g | já com o giro de 45° que desvia do funil |

Total ≈ **256 g** (28BYJ) ou **263 g** (NEMA 17). Cabe tudo em duas mesas na K1 Max (300×300).
Sugestão: mesa 1 = `bracket` + `hopper` + `lid`; mesa 2 = o resto.

## Detalhes que importam

**`rosca.stl` (a rosca)** — imprime em pé. A versão do NEMA 17 tem 81 mm de altura
para 14 mm de diâmetro: **use brim** (5 mm) e velocidade de 40 mm/s, senão ela
descola no meio da impressão. A hélice é uma escada helicoidal:
cada camada apoia na anterior, então sai sem suporte. Se a borda da hélice
ficar “peluda”, baixe a velocidade para 40 mm/s só nessa peça. Depois de
imprimir, passe o dedo na hélice para tirar rebarba — rebarba é a causa nº 1
de rosca travando no tubo.

**`tubo_metade_a` / `tubo_metade_b`** — saem com a face plana na mesa, canal para
cima. A metade **A** tem furos de 2,55 mm (o parafuso M3 rosca direto no
plástico); a **B** tem furos passantes de 3,3 mm com rebaixo para a cabeça. Não
troque.

Com **NEMA 17** o tubo é 13 mm mais longo (o cubo da rosca precisa engolir o eixo
longo do motor) e o prato de fixação é 52 × 52 mm com os 4 furos em quadrado de
31 mm. Fora isso é a mesma peça.

**`bracadeira.stl`** — é um perfil constante extrudado 40 mm: imprime em pé, sem
nenhum overhang. Parece muito material (66 g) porque é a peça que segura ~400 g
pendurados na borda do aquário.

**`funil.stl`** — parede de 2,2 mm com 3 perímetros fica translúcido, o que é
ótimo: você vê o nível da ração. As paredes têm 72° de inclinação para a ração
escorrer sozinha.

Se quiser um funil maior/menor, mude `hop_taper_h` e `hop_top` em
`cad/common.scad` (o socket de baixo precisa continuar com 25,6 mm).
