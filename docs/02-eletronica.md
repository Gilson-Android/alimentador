# 2. Eletrônica

> **Esquema visual colorido:** [guia no site → Eletrônica](https://gilson-android.github.io/alimentador/#eletronica).
> Lá tem o diagrama de ligação desenhado, com as cores de cada fio. Este arquivo é
> a versão em texto, para consultar offline.

O projeto suporta dois motores. Escolha a sua seção.

---

## Opção A — 28BYJ-48 + ULN2003 (5 V)

São 6 fios. Não precisa soldar nada se usar jumpers fêmea-fêmea.

```
      FONTE 5V 2A
        +5V  ────────────┬──────────────┬─────────┐
                         │              │         │
        GND  ────────┬───┼──────────┬───┼─────────┼──┐
                     │   │          │   │         │  │
                  ESP32-S3        ULN2003      C1 470–1000uF
                  5V   GND        5V   GND     (+)      (−)

   ESP32-S3 GPIO 1  ──────────────►  IN1   ULN2003
   ESP32-S3 GPIO 2  ──────────────►  IN2
   ESP32-S3 GPIO 41 ──────────────►  IN3
   ESP32-S3 GPIO 42 ──────────────►  IN4

   ULN2003 (conector branco 5 vias) ──► motor 28BYJ-48
   Câmera OV2640 ──► conector flat da própria placa
```

| ULN2003 | ESP32-S3 | Por quê |
|---|---|---|
| IN1 | GPIO 1 | |
| IN2 | GPIO 2 | |
| IN3 | GPIO 41 | |
| IN4 | GPIO 42 | |
| 5 V | +5 V da fonte | motor puxa até ~500 mA, não tire do 3V3 |
| GND | GND comum | **o GND da fonte, do ESP32 e do driver tem que ser o mesmo** |

Compile com `pio run -e freenove_s3_cam -t upload`.

---

## Opção B — NEMA 17 + DRV8825 (12 V)

```
   FONTE 12V ──┬── VMOT (driver) ── C 100uF entre VMOT e GND
               └── step-down 5V ──► ESP32-S3 (pino 5V)

   GND da fonte ──► GND do ESP32 ──► GND do driver   (tudo no mesmo GND)

   ESP32 GPIO 1  ──► STP         M0 ──┐
   ESP32 GPIO 2  ──► DIR         M1 ──┴─► 3V3   (1/8 de micropasso)
   ESP32 GPIO 41 ──► EN          M2 ────► GND
   ESP32 3V3     ──► RST + SLP
   1A + 2A ──► uma bobina do motor   (pinos vizinhos na placa)
   1B + 2B ──► a outra bobina
```

Os nomes abaixo são os da **serigrafia da placa** (o clone roxo da Pololu):

| Pino (serigrafia) | Ligar em | Observação |
|---|---|---|
| **STP** | GPIO 1 | é o STEP |
| **DIR** | GPIO 2 | |
| **EN** | GPIO 41 | ativo em nível baixo; o firmware solta o motor quando para |
| **M0**, **M1** | 3V3 do ESP32 | M2=0 M1=1 M0=1 → **1/8 de micropasso** |
| **M2** | GND | idem |
| **RST**, **SLP** | 3V3 do ESP32 | os dois; sem isso o driver fica em reset |
| **VMOT** + o **GND** vizinho | +12 V da fonte | **100 µF eletrolítico direto nesses dois pinos** |
| **GND** (o outro) | GND comum | mesmo GND do ESP32 e da fonte |
| **1A** + **2A** | uma bobina do motor | os dois pinos de cada bobina são **vizinhos** na placa |
| **1B** + **2B** | a outra bobina | |
| **FLT** | — | é o FAULT; deixe solto |

Os 4 pinos de motor ficam no meio daquela fileira, entre o par `VMOT`/`GND` e o
par `FLT`/`GND`. **Contando a partir do lado do `VMOT`**, a ordem é:

```
VMOT · GND · 2B · 1B · 1A · 2A · FLT · GND
              └──┬──┘   └──┬──┘
              bobina 1  bobina 2
```

Os dois pinos de cada bobina são vizinhos — é isso que faz o plugue do cabo
encaixar direto.

**O DRV8825 não tem pino VDD.** Diferente do A4988, ele gera a lógica dele
internamente a partir do VMOT — não existe fio de 3,3 V *alimentando* o driver. O
3V3 do ESP32 serve só para puxar M0, M1, RST e SLP para nível alto. Consequência:
**sem os 12 V o driver está morto**, mesmo com o ESP32 ligado.

### Limite de corrente — faça antes de ligar o motor

Com o motor **desconectado** e os 12 V ligados, meça a tensão entre o cursor do
trimpot e o GND:

- **DRV8825:** `Vref = corrente ÷ 2` → **0,25 V para 0,5 A**
- A4988: `Vref = corrente × 0,8` → 0,4 V para 0,5 A

0,4 a 0,6 A é mais que suficiente para a rosca e mantém o driver frio.

### O cabo que vem com o motor já resolve os pares

Os cabos de motor de impressora **trocam a ordem dos fios entre as duas pontas**,
de propósito:

| Ponta | Ordem | Pares |
|---|---|---|
| no motor (JST) | vermelho · preto · azul · verde | intercalados |
| no driver (plugue 4 vias) | vermelho · azul · verde · preto | **vizinhos** |

Como os 4 pinos de motor do DRV8825 também são vizinhos por par (`2B`+`1B` e
`1A`+`2A`), o plugue **encaixa direto nos 4 pinos em sequência**, sem separar fio
nenhum.

E tanto faz a orientação: plugado virado 180° os pares continuam certos (só troca
qual bobina é qual). A única diferença é o sentido de giro da rosca, que se
corrige em *Ajustes → Dosagem → Inverter sentido*, sem mexer em fio.

### Se o seu cabo for outro: achar os pares

Multímetro em continuidade (ou ohms) e toque dois fios do motor por vez. O par que
apitar — ou marcar 2 a 4 Ω — é uma bobina, e vai em **1A + 2A**. O outro par vai em
**1B + 2B**.

Em motor de impressora com fios **vermelho / preto / azul / verde**, o mais comum é
**preto + verde** numa bobina e **vermelho + azul** na outra. Mas clone é clone: meça.

Se o motor vibrar no lugar sem girar, você misturou os pares — troque **2A** com **1B**.

### Outros drivers

| Driver | Micropasso 1/8 | Precisa de VDD? |
|---|---|---|
| DRV8825 | M0=H, M1=H, M2=L | não |
| A4988 | MS1=H, MS2=H, MS3=L | **sim**, 3V3 |
| TMC2208/2209 | MS1=L, MS2=L (padrão) | sim, 3V3 |

Compile com `pio run -e freenove_s3_cam_nema17 -t upload`.

O DRV8825 esquenta: deixe o dissipador virado para as fendas de ventilação da
caixa e não encoste nada nele. O step-down também aquece — mantenha os dois
separados.

---

## O capacitor não é opcional (vale para os dois motores)

O motor de passo dá picos de corrente a cada passo. Sem o eletrolítico ligado
entre a alimentação e o GND **junto do driver**, o pico derruba a tensão e você
vê: câmera falhando ao inicializar, reset aleatório, Wi-Fi caindo exatamente na
hora de alimentar. Respeite a polaridade — a faixa clara no corpo marca o
negativo.

- 28BYJ-48 / ULN2003: **470–1000 µF** entre 5 V e GND
- NEMA 17 / DRV8825: **100 µF** entre VMOT e GND

## Sensor de grãos (opcional, mas recomendado para viagem)

Confirma que a ração realmente caiu. Sem ele, o alimentador "acha" que alimentou
mesmo se a rosca estiver travada ou o funil vazio.

- Imprima a calha na variante **`calha_com_sensor.stl`**: ela tem dois furos de
  3,4 mm alinhados, um de cada lado do duto. Encaixe por pressão um **LED IR de
  3 mm** num furo e um **fototransistor de 3 mm** no outro — o feixe atravessa
  o duto e cada grão que passa gera um pulso. (Um módulo **TCRT5000**
  reflexivo, com os 3 pinos VCC/GND/D0, também funciona: aponte para o furo.)
- Alimente com **3,3 V** (não 5 V — a entrada do ESP32 não é 5 V tolerante).
  Com LED+fototransistor discretos: resistor de 150 Ω em série no LED e
  10 kΩ do coletor para 3,3 V.
- Saída digital → **GPIO 21**.
- Ligue em *Ajustes → Sensor de grãos* e **reinicie** o aparelho.

Se o sensor estiver ligado e nenhum grão for detectado, o firmware recua a
rosca, tenta de novo e — se ainda nada — registra falha no histórico e manda
aviso no Telegram.

## Alimentação

**28BYJ-48:** um carregador de celular de 5 V / 2 A resolve. Ligue placa e driver
na mesma fonte, em paralelo, com fio de 22 AWG ou mais grosso.

**NEMA 17:** fonte de 12 V / 2 A + um step-down (MP1584 ou LM2596) para tirar os
5 V do ESP32. **Ajuste a saída do step-down para 5,0 V antes de ligar na placa** —
esses módulos saem de fábrica em qualquer tensão.

Não use powerbank como fonte fixa: muitos desligam sozinhos quando o consumo cai
(o alimentador fica em repouso a maior parte do tempo).

**Nobreak:** se sua região tem queda de luz frequente, vale plugar a fonte num
nobreak pequeno. Mas o firmware já recupera o horário perdido quando a energia
volta — veja `catchUpMin` em Ajustes.

## Montagem dentro da caixa

A caixa (`caixa_eletronica.stl`) tem:

- pinos para a placa ESP32-S3 (esquerda) e para o driver (direita) — para
  parafuso auto-atarraxante M2,5;
- **fendas de abraçadeira** como plano B: se os furos da sua placa não baterem
  com os pinos, prenda com abraçadeira de nylon ou fita dupla face e ignore os pinos;
- janela para o USB-C numa das pontas (essa ponta fica para **cima** na montagem);
- dois furos de 7,5 mm na outra ponta para os cabos saírem **para baixo**;
- 2 furos de 4,5 mm no fundo para parafusar no mastro da braçadeira.

> **Confira com paquímetro** a distância entre os furos da sua placa e ajuste
> `b1_hx` / `b1_hy` em `cad/common.scad` antes de imprimir a caixa. Os valores
> padrão (55 × 20 mm) são para a placa Freenove.

**Laço de gotejamento:** deixe os cabos saírem pela parte de baixo da caixa e
subirem em curva até o motor. Se um dia escorrer água pelo cabo, ela pinga no
chão em vez de entrar na caixa.
