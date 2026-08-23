# 2. Eletrônica

São 6 fios. Não precisa soldar nada se usar jumpers fêmea-fêmea.

## Ligações (placa padrão: Freenove ESP32-S3-WROOM CAM)

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

### O capacitor não é opcional

O 28BYJ-48 dá picos de corrente a cada passo. Sem um eletrolítico de
**470–1000 µF** ligado entre 5 V e GND **junto do ULN2003**, o pico derruba a
tensão e você vê: câmera falhando ao inicializar, reset aleatório, Wi-Fi caindo
na hora de alimentar. Respeite a polaridade (perna comprida = +).

### Sensor de grãos (opcional, mas recomendado para viagem)

Confirma que a ração realmente caiu. Sem ele, o alimentador "acha" que alimentou
mesmo se a rosca estiver travada ou o funil vazio.

- Imprima a calha na variante **`chute_com_sensor.stl`**: ela tem dois furos de
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
rosca 400 passos, tenta de novo e — se ainda nada — registra falha no histórico
e manda aviso no Telegram.

## Alimentação

Um carregador de celular de 5 V / 2 A resolve. Ligue os dois consumidores
(placa e driver) **na mesma fonte**, em paralelo, com fio de 22 AWG ou mais grosso.

Não use powerbank como fonte fixa: muitos desligam sozinhos quando o consumo
cai (o alimentador fica em repouso a maior parte do tempo).

**Nobreak:** se sua região tem queda de luz frequente, vale plugar a fonte num
nobreak pequeno. Mas o firmware já recupera o horário perdido quando a energia
volta — veja `catchUpMin` em Ajustes.

## Montagem dentro da caixa

A caixa (`box_base`) tem:

- pinos para a placa ESP32-S3 (esquerda) e para o ULN2003 (direita) — para
  parafuso auto-atarraxante M2,5;
- **fendas de abracadeira** como plano B: se os furos da sua placa não baterem
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
