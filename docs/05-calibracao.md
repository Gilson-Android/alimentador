# 5. Calibração da dose

O firmware não sabe o que é "uma porção" para o *seu* peixe. Isso se resolve em
10 minutos e é o passo que mais influencia o resultado final.

## Quanto o peixe come, de verdade

Regra prática: a quantidade que os peixes limpam em **2 a 3 minutos**, sem
sobrar nada no fundo. Para a maioria dos aquários domésticos (10 a 60 L,
peixes pequenos) isso dá algo entre **0,2 e 0,6 ml por refeição** — bem menos
do que a intuição sugere.

Alimente **menos** que o normal por vez e use dois horários em vez de um só.
Ração que sobra vira amônia, e amônia mata.

## Passo a passo

1. Ponha ração no funil (pelo menos 1/3, para o peso ajudar a descer).
2. Encha a rosca: *Agora → Manutenção → Avançar* umas 3 vezes, até começar a
   cair ração de forma regular. Recolha essa ração num potinho, não jogue no aquário.
3. Ponha um papel/potinho embaixo da calha.
4. Clique em **1 porção** e olhe quanto caiu.
5. Ajuste em *Ajustes → Dosagem → Passos por porção*:

| Objetivo | Passos por porção | Dose aproximada |
|---|---|---|
| bem pouco | 512 | ~0,12 ml |
| **padrão** | **1024** | **~0,23 ml** |
| médio | 2048 (meia volta) | ~0,46 ml |
| bastante | 4096 (uma volta) | ~0,93 ml |

6. Repita 5 vezes seguidas e veja se a dose é **repetível**. Variação de ±20% é
   normal com ração granulada; mais que isso indica ração muito grande para a
   rosca ou hélice com rebarba.

## Depois de calibrar, ajuste as travas

Em *Ajustes → Segurança*:

- **Máx. porções/dia** — ponha o total real que você usa por dia + 2 de margem.
  Isso é o que impede que uma falha, um duplo clique ou um comando repetido de
  Telegram entupam o aquário de comida. Padrão: 10.
- **Máx. por vez** — 2 ou 3 é suficiente.
- **Intervalo mínimo** — 45 s evita clique duplo acidental.
- **Janela de recuperação** — 90 min. Se a luz voltar depois disso, o horário
  perdido é ignorado (melhor um jejum que uma dose fora de hora).

## Ração: o que funciona

| Tipo | Funciona? |
|---|---|
| Granulado / pellet até 3 mm | Sim, é o ideal |
| Micro pellet, ração de alevino | Sim |
| Flocos pequenos e secos | Geralmente sim |
| Flocos grandes | Amasse antes, senão trava |
| Ração úmida, liofilizado leve | Não use |

A rosca tem folga radial de 4 mm entre o núcleo e a parede — grãos maiores que
isso vão encravar. Se sua ração é grande e você não quer amassar, aumente
`auger_od`, `auger_core` e `auger_pitch` em `cad/common.scad` e reimprima rosca
e tubo (o resto continua igual).

## Umidade: o inimigo de verdade

Ração absorvendo umidade empedra e trava a rosca. Isso é o motivo nº 1 de
alimentador automático falhar em viagem.

- Sachê de **sílica gel** dentro do funil, sempre.
- Tampa fechada, sempre.
- Não encha o funil até a boca: ração velha no fundo é a que empedra. Encha o
  que der para 3–4 semanas.
- Se seu aquário é aberto e o ambiente é úmido, considere trocar a ração do
  funil antes de cada viagem.
