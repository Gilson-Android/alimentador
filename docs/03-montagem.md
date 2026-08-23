# 3. Montagem

![Vista explodida](img/explodido.png)

Como fica por dentro (corte no meio do tubo):

![Corte](img/corte.png)

## Parafusos usados

| Onde | Parafuso | Qtd |
|---|---|---|
| Metades do tubo (orelhas) | M3 × 8 | 3 |
| Motor no prato — 28BYJ-48 | M3 × 10 | 2 |
| Motor no prato — NEMA 17 | M3 × 10 | 4 |
| Funil no colar do tubo | M3 × 12 | 2 |
| Tampa do berço na braçadeira | M3 × 16 | 2 |
| Tampa da caixa | M3 × 10 | 4 |
| Caixa no mastro | M4 × 12 | 2 |
| Suporte da câmera na caixa | M3 × 12 | 2 |
| PCB da câmera no suporte | M2 × 6 | 2 |
| Aperto da braçadeira | M4 × 30 + porca M4 | 1 |

Total: **11 × M3 curtos, 2 × M3 × 16, 2 × M4 × 12, 2 × M2 × 6, 1 × M4 × 30 + porca**.
Um kit variado de M3 de bicicleta/eletrônica resolve.

Tudo que rosca no plástico rosca **direto** (sem inserto): entre devagar, sem
apertar até o fim da força.

## Passo 1 — rosca no tubo

1. Limpe a rebarba da hélice da rosca (`auger`) com um estilete.
2. Encaixe o eixo do motor no cubo da rosca:
   - **28BYJ-48:** o eixo é achatado dos dois lados e entra com pressão firme dos
     dedos. Duro? Lima fina. Frouxo? Uma gota de cola instantânea.
   - **NEMA 17:** o eixo é redondo e liso. Ele entra justo no cubo (que é fundo o
     bastante para engolir os 20–25 mm de eixo, sem precisar cortar nada) e depois
     você trava com o **parafuso M3 radial**. Dica: lime um pequeno chanfro no
     eixo no ponto onde o parafuso encosta — aí ele não escorrega nunca mais.
3. Deite a rosca na metade **B** do tubo, com a ponta fina (mancal traseiro) no
   fundo da bolsa e o cubo do lado do prato do motor.
4. Feche com a metade **A** e aperte os **3 M3 × 8** nas orelhas
   (2 na frente, 1 atrás). A cabeça entra pelo lado B.
5. **Teste antes de continuar:** gire o eixo do motor com a mão. A rosca tem que
   girar livre, com o mínimo de arrasto. Se travar, afrouxe os parafusos, alinhe
   e aperte de novo — ou lime a hélice.

## Passo 2 — motor

**28BYJ-48:** parafuse no prato com **2 M3 × 10** (roscam direto no plástico, entre
devagar), com o corpo do motor **para baixo**. O ressalto do eixo entra no rebaixo
do prato — é isso que garante o alinhamento.

**NEMA 17:** parafuse com **4 M3 × 10** no quadrado de 31 mm. Os parafusos entram
pelo lado de dentro (entre o prato e o tubo) e roscam na carcaça do próprio motor,
então use chave allen com ponta esférica. O ressalto de 22 mm do motor entra no
rebaixo do prato.

## Passo 3 — funil

1. Encaixe o spigot (a saia quadrada) do funil no socket do tubo.
2. Parafuse os **2 M3 × 12** do flange nas orelhas do colar. Esses dois
   parafusos também prendem as duas metades do tubo na região do colar.
3. Jogue um sachê de sílica gel dentro do funil antes de pôr ração.

## Passo 4 — calha

Encaixe a calha (`chute`) empurrando por baixo no boss do tubo até as duas
covinhas estalarem nas meias-esferas. Ela sai com um puxão firme, para limpeza.
A boca inclinada da calha aponta **para dentro do aquário**.

## Passo 5 — braçadeira

1. Encaixe a porca M4 no alojamento sextavado da garra externa (se ficar
   folgada, uma gota de cola).
2. Rosqueie o M4 × 30 e encaixe o manípulo na cabeça.
3. Apoie a braçadeira na borda do aquário, deite o tubo no berço e feche com a
   tampa do berço + **2 M3 × 16**. Onde o berço abraça o tubo depende do motor
   (é a mesma peça, só muda a posição):
   - **28BYJ-48:** berço em x = 11 a 49 mm, tampa em x = 40 a 49 mm
   - **NEMA 17:** berço em x = 24 a 62 mm, tampa em x = 53 a 62 mm
4. Aperte o manípulo até firmar na borda. **Não precisa de força de macaco** —
   plástico racha.

Confira: a saída da calha tem que ficar **acima da linha d'água**, uns 2 a 5 cm.
Se no seu aquário a água está muito alta ou muito baixa, ajuste `mast_h` em
`cad/bracket.scad` e reimprima a braçadeira (é a única peça afetada).

> **Aquário com tampa?** Fica mais firme apoiar o conjunto na tampa e fazer um
> furo de ~25 mm para a calha passar. Nesse caso você não precisa da braçadeira
> nem da tampa do berço.

## Passo 6 — eletrônica e câmera

1. Fixe as placas na caixa (pinos, abraçadeira ou dupla face).
2. Faça as ligações da [seção 2](02-eletronica.md), com o capacitor junto do ULN2003.
3. Parafuse a caixa no mastro (2 × M4 × 12, roscam no plástico), com a ponta do USB **para cima**.
4. Monte a PCB da câmera no `cam_pod` com 2 M2 × 6 e parafuse o pod em cima da caixa.
5. Passe o cabo flat da câmera com cuidado — ele é frágil e o conector tem
   trava (levante a aba escura, insira, abaixe a aba).
6. Cabos do motor e da fonte saem pela **parte de baixo** da caixa, em laço.

O suporte da câmera tem **duas** inclinações embutidas: `cam_tilt` (45° para
baixo) e `cam_yaw` (45° na horizontal). O *yaw* existe porque olhando reto a
câmera ficaria de cara com a parede do funil — girada 45° ela passa ao lado e
enquadra a superfície da água e a saída da calha:

![O que a câmera vê](img/vista_da_camera.png)

Se a imagem sair de cabeça para baixo ou espelhada, corrija em
*Ajustes → Câmera* (girar/espelhar) — não precisa reimprimir. Se quiser mudar
para onde ela aponta, mexa em `cam_tilt` / `cam_yaw` em `cad/cam_pod.scad` e
reimprima (15 g). O pod é simétrico: montando virado 180° na caixa, ele olha
para o outro lado do aquário.

## Passo 7 — primeiro teste, ainda sem ração

Ligue, configure o Wi-Fi ([seção 4](04-firmware.md)) e use
*Agora → Manutenção da rosca → Avançar*. A rosca deve girar suave, sem barulho
de trava e sem esquentar o motor além de "morno".

Só depois ponha ração e vá para a [calibração](05-calibracao.md).
