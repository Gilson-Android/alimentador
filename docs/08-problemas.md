# 8. Quando dá problema

## Mecânica

**A rosca trava / range.**
Rebarba na hélice (lime), parafusos das metades apertados demais (desaperte e
alinhe pelo giro livre), ou grão de ração maior que a folga. Teste sem ração:
se gira livre vazia e trava cheia, é a ração.

**Sai muito mais ração que o programado.**
Ração fluindo por gravidade pelo canal. O tubo é horizontal justamente para
evitar isso — confira se a braçadeira não deixou o conjunto inclinado para
frente. Reduza também a folga: `radial_clear = 0.3` em `common.scad`.

**Sai pouco ou nada, mas o motor gira.**
Ração empedrada fazendo "ponte" na boca do funil. Bata de leve no funil: se
descer, é isso. Troque a ração, ponha sílica e considere aumentar
`ANTIJAM_STEPS` em `firmware/include/config.h`.

**Motor esquenta.**
Normal ficar morno. Se estiver quente, aumente os µs por passo (1300 → 2000) e
confira a fonte: 5 V com queda para 4,2 V faz o motor perder torque e esquentar.
As bobinas são desligadas quando o motor para — se estiver esquentando
*parado*, tem pino travado em nível alto: revise a fiação IN1..IN4.

**Motor vibra mas não gira.**
Fios do motor na ordem errada no ULN2003, ou dois pinos IN trocados. Teste
com *Manutenção → Avançar*: se ele "tremer" no lugar, troque IN3 com IN4.

## Eletrônica

**Resets aleatórios ao alimentar.**
Falta o capacitor de 470–1000 µF junto do ULN2003, fonte fraca, ou fio fino
demais. É o problema mais comum do projeto.

**A câmera não inicializa (`esp_camera_init 0x105`).**
Cabo flat mal encaixado ou invertido, ou pinagem errada para a sua placa.
Confira `firmware/include/camera_pins.h` contra o exemplo oficial do fabricante
da SUA placa. Sem PSRAM a câmera cai para 640×480 automaticamente.

**Imagem verde/listrada.**
Alimentação instável ou cabo flat comprido demais. Baixe a resolução em
*Ajustes → Câmera*.

## Rede

**Não acho o `aquafeeder.local`.**
mDNS não funciona em algumas redes e em Android antigo. Use o IP direto (aparece
no monitor serial ou na lista de dispositivos do roteador) e reserve o IP no DHCP.

**Vídeo travando / interface lenta com o vídeo ligado.**
Normal: o stream MJPEG consome quase toda a banda do Wi-Fi do ESP32. Use *Foto*
no dia a dia e ligue o vídeo só quando quiser olhar. O stream roda na porta 81
justamente para não travar a API.

**Wi-Fi cai e não volta.**
O firmware tenta reconectar a cada 15 s e reinicia sozinho após 10 min sem rede.
Se cai sempre, o sinal está ruim: aproxime o roteador ou use um repetidor.
Veja o `rssi` em `/api/status` — abaixo de −75 dBm é problema.

**Esqueci a senha da interface.**
Grave o firmware por USB de novo com a NVS apagada:
```bash
pio run -e freenove_s3_cam -t erase && pio run -e freenove_s3_cam -t upload
```
Isso apaga TODA a configuração (Wi-Fi, agenda, contadores).

## Agenda e horários

**Alimentou na hora errada.**
Fuso horário. Em *Ajustes*, `Fuso (TZ POSIX)` tem que ser `<-03>3` para o
Brasil. Confira o relógio na aba *Agora*: se mostrar "sem hora", o NTP não
sincronizou (sem internet, ou o roteador bloqueia UDP 123).

**Não alimentou no horário.**
Cheque nesta ordem: (1) o slot está marcado como ativo e com o dia da semana
certo; (2) o relógio está sincronizado; (3) o limite diário não foi atingido —
o *Histórico* e o log serial mostram "slot barrado: Limite diário atingido".

**Alimentou duas vezes.**
Não deveria acontecer: cada horário só dispara se a última refeição foi antes
dele. Se aconteceu, provavelmente foram dois slots em horários próximos, ou
alguém clicou no botão além da agenda — o *Histórico* mostra a origem de cada
refeição.

## Log serial

É a melhor ferramenta de diagnóstico:

```bash
cd firmware && pio device monitor
```

O que procurar:

```
[cfg] carregado (412 bytes)
[feeder] pronto. 1024 passos/porcao, 1300 us/passo, hoje=2
[cam] ok (psram=1, tam=2, q=12)
[wifi] ok  ip=192.168.0.42  rssi=-58 dBm
[web] http://aquafeeder.local (porta 80)
[tg] pronto, offset inicial 123456789
[agenda] slot 0 disparado (08:00)
[feeder] 2 porcao(oes) via agenda, graos=14, ok=1
```
