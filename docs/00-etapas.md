# 0. Roteiro — as 9 etapas, nesta ordem

Cada etapa termina num **teste objetivo**. Não passe para a próxima sem o visto:
é assim que você descobre um problema enquanto ele ainda é pequeno e barato.

A câmera é a última de propósito. Ela é a única parte que, se não funcionar,
**não impede o alimentador de alimentar** — e é a que mais dá trabalho. Colocar
ela no começo é o jeito mais rápido de travar o projeto inteiro num detalhe que
não é essencial.

| # | Etapa | Termina quando |
|---|---|---|
| 1 | Peça de teste | os 4 encaixes conferem |
| 2 | Motor girando na bancada | gira suave nos dois sentidos, sem esquentar |
| 3 | Wi-Fi, relógio e senha | `aquafeeder.local` abre e mostra a hora certa |
| 4 | Mecânica seca | a rosca gira livre no tubo, sem ração |
| 5 | Dosagem | 5 doses seguidas com variação ≤ 20% |
| 6 | Montado no aquário | a ração cai na água livre, calha acima do nível |
| 7 | Agenda | 7 dias rodando sozinho, sem sobra no fundo |
| 8 | Acesso remoto | `/status` responde pelo 4G do celular |
| 9 | Câmera | a foto mostra a água e a saída da calha |

---

## Etapa 1 — Peça de teste

**Faz:** imprime `stl/motor_nema17/peca_de_teste.stl` (ou `motor_28byj/`) com o
mesmo filamento e as mesmas configurações que vai usar no resto.

**Visto quando:** o motor parafusa nos 4 furos e assenta plano; o eixo entra no
cilindro deslizando, sem folga de girar; o quadrado menor entra no maior com
atrito de dedo.

**Se falhar:** mexa só em `fit` no `cad/common.scad` (`0.10` afrouxa tudo,
`-0.10` aperta), rode `cad/build.sh` e reimprima **só a peça de teste**.

---

## Etapa 2 — Motor girando na bancada

Ainda **sem nada de mecânica**. Motor solto na mesa.

**Faz:**
1. Liga ESP32 + driver + motor conforme a [seção 2](02-eletronica.md).
2. **Ajusta o Vref com o motor desconectado**: 0,25 V para 0,5 A no DRV8825.
3. Grava: `pio run -e devkitc_s3_n16r8_nema17 -t upload`.
4. Conecta na rede `AquaFeeder-XXXX` (senha `aquafeeder`), abre
   http://192.168.4.1 e usa *Agora → Manutenção → Avançar* e *Recuar*.

**Visto quando:** o eixo gira suave nos dois sentidos, o motor fica só morno e o
driver também. Sem trancos, sem chiado agudo.

**Se falhar:**

| Sintoma | Causa provável |
|---|---|
| vibra no lugar, não gira | pares de bobina trocados — inverta `2A` com `1B` |
| não faz nada | `RST`+`SLP` sem 3V3, ou `VMOT` sem 12 V (o DRV8825 morre sem os 12 V) |
| gira mas trava fácil | Vref baixo demais |
| esquenta muito | Vref alto demais, ou µs por passo pequeno demais |
| ESP reinicia ao girar | falta o capacitor de 100 µF no `VMOT` |

---

## Etapa 3 — Wi-Fi, relógio e senha

**Faz:** em *Ajustes → Rede e sistema*, preenche o seu Wi-Fi, **define uma senha
para a interface** e salva. O aparelho reinicia. Depois reserva o IP dele no
roteador (DHCP estático).

**Visto quando:** `http://aquafeeder.local` abre de outro aparelho da casa **e** o
relógio na aba *Agora* mostra a hora certa.

> Se o relógio mostrar **"sem hora"**, pare aqui: a agenda **não dispara** sem
> relógio sincronizado. Confira o fuso (`<-03>3`) e se a rede tem internet.

---

## Etapa 4 — Mecânica seca

**Faz:** imprime e monta tubo + rosca + funil + calha ([seção 3](03-montagem.md)).
Ainda **sem ração** e **fora do aquário**.

**Visto quando:** você gira o eixo do motor com a mão e a rosca gira livre; no
*Avançar* não há barulho de raspagem; depois de 3 voltas o motor continua morno.

**Se falhar:** rebarba na hélice (lima), parafusos das metades apertados demais
(afrouxe, alinhe pelo giro livre, reaperte), ou barriga no teto do furo do tubo
(raspe com uma lixa enrolada).

---

## Etapa 5 — Dosagem

**Faz:** ração no funil, potinho embaixo da calha, ainda fora do aquário.
Enche a rosca com 3 *Avançar* (jogue essa ração fora), depois clica *1 porção*
cinco vezes e compara.

**Visto quando:** as 5 doses são parecidas — variação de até 20% é normal com
granulado. Ajuste *Ajustes → Passos por porção* até a dose bater com o que seus
peixes limpam em 2–3 minutos.

**Depois:** aperte as travas em *Ajustes → Segurança* (máx./dia justo, não
generoso). Ver [seção 5](05-calibracao.md).

---

## Etapa 6 — Montado no aquário

**Faz:** braçadeira na borda, tubo no berço, caixa no mastro, cabos saindo por
baixo em laço de gotejamento.

**Visto quando:** a saída da calha fica **2 a 5 cm acima da linha d'água**; você
solta 1 porção e a ração cai em **água livre** — não no vidro, não sobre planta,
não em canto onde acumula.

**Se falhar:** ajuste `mast_h` em `cad/bracket.scad` e reimprima só a braçadeira,
ou apoie o conjunto na tampa do aquário com um furo de 25 mm para a calha.

---

## Etapa 7 — Agenda

**Faz:** dois horários na aba *Agenda* e **deixa rodando 7 dias com você em
casa**. No meio da semana, desligue a fonte durante um horário programado e
religue 10 minutos depois.

**Visto quando:** o *Histórico* mostra todos os disparos; **nada sobra no fundo
do aquário**; e o teste de queda de energia alimentou ao voltar.

Este é o único teste que realmente vale. Sem essa semana, você não sabe se o
alimentador funciona — sabe que ele funcionou uma vez.

---

## Etapa 8 — Acesso remoto

**Faz:** Telegram (mais simples) e/ou MQTT (para integrar com outros sistemas).
Ver [seção 6](06-remoto.md).

**Visto quando:** `/status` e `/alimentar` respondem com o celular no **4G**, não
no Wi-Fi de casa. Testar pelo Wi-Fi não prova nada.

**Bônus com MQTT:** o tópico `online` avisa sozinho se o alimentador cair — é o
alarme que faz diferença quando você está longe.

---

## Etapa 9 — Câmera (última)

**Faz:** escolhe um caminho:

1. **Módulo OV2640** ligado nos 14 pinos ([seção 2](02-eletronica.md)) — fios de
   até 10 cm, e se a imagem sair listrada compile com `-DCAM_XCLK_HZ=10000000`.
2. **Celular velho** com o app *IP Webcam*, e a URL em *Ajustes → Câmera →
   Câmera externa*. Sem fio, imagem melhor.

**Visto quando:** a foto na aba *Agora* mostra a **superfície da água** e a
**saída da calha** no mesmo quadro. É esse enquadramento que serve para
diagnosticar de longe: você vê se caiu ração e se a água está boa.

**Se falhar:** o alimentador continua funcionando. Câmera é conforto, não
requisito — e existe o plano B do celular.

---

## Antes de viajar

Com as 9 etapas fechadas, vá para o [checklist de viagem](07-antes-de-viajar.md).
Ele cobre o que fazer nas duas semanas, na semana e no dia anterior — e o plano B
com uma pessoa de confiança.
