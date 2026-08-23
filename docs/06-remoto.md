# 6. Usar de fora de casa

A interface web só existe dentro da sua rede. Para acionar de outro lugar você
tem quatro caminhos — em ordem de recomendação:

## Opção 1 — Telegram (mais simples e mais segura)

Não precisa abrir nada no roteador: é o alimentador que sai para conversar com
o Telegram. Funciona de qualquer lugar do mundo, no celular que você já usa.

### Criar o bot

1. No Telegram, fale com **@BotFather** → `/newbot` → escolha um nome.
2. Ele devolve um token tipo `7891234567:AAE...`. **Esse token é a chave da sua
   casa** — não compartilhe, não poste em fórum.
3. Cole em *Ajustes → Telegram → Token do bot* e salve.
4. Mande qualquer mensagem para o seu bot. Ele responde com o seu **chat id**.
5. Cole esse número em *Ajustes → Telegram → Chat ID autorizado* e salve.

Pronto. Só esse chat id consegue dar comandos; qualquer outra pessoa que
encontre o bot é ignorada.

### Comandos

| Comando | O que faz |
|---|---|
| `/alimentar` | 1 porção |
| `/alimentar 2` | 2 porções |
| `/foto` | tira e envia uma foto do aquário |
| `/status` | porções do dia, última refeição, sinal do Wi-Fi |
| `/agenda` | lista os horários programados |

Com *Avisar a cada alimentação* ligado, você recebe uma mensagem a cada
refeição (inclusive as da agenda) — e um alerta se o sensor de grãos não
detectar ração caindo.

**Sobre segurança:** a conexão com o Telegram é HTTPS, mas o firmware **não
valida o certificado** (`setInsecure()`) para economizar memória — é o que
praticamente toda biblioteca de bot para ESP32 faz. O risco real é baixo (só
alguém já dentro da sua rede poderia se passar pelo Telegram), mas está dito.

## Opção 2 — MQTT (para integrar com outros sistemas)

Se você quer comandar de fora do celular (n8n, Home Assistant, Power Automate,
script), use MQTT. O ESP32 conecta **para fora** no broker e fica escutando
`aquafeeder/cmd` — nenhuma porta aberta no roteador.

Configure em *Ajustes → MQTT*: broker, porta, usuário, senha e prefixo. Broker
vazio desliga. Funciona com qualquer broker padrão; HiveMQ Cloud e EMQX
Serverless têm plano free com TLS, e Mosquitto em casa roda sem TLS na 1883.

| Tópico | Direção | Conteúdo |
|---|---|---|
| `aquafeeder/cmd` | você → aparelho | `feed 2`, `jog -800`, `status`, `reboot` (ou JSON) |
| `aquafeeder/online` | aparelho → você | `1`/`0` retido — o `0` vem do **broker** (Last Will) |
| `aquafeeder/status` | aparelho → você | JSON retido, a cada 60 s e a cada evento |
| `aquafeeder/evento` | aparelho → você | uma mensagem por alimentação |

**O que o MQTT dá e o Telegram não dá:** alarme automático de "parou de
funcionar". Se o alimentador travar, cair a luz ou cair a internet, é o próprio
broker que publica `online = 0`. Com Telegram você recebe silêncio — que é
indistinguível de "está tudo bem e ninguém mandou mensagem".

```bash
mosquitto_pub -h SEU-BROKER -p 8883 -u user -P senha -t aquafeeder/cmd -m "feed 2"
mosquitto_sub -h SEU-BROKER -p 8883 -u user -P senha -t 'aquafeeder/#' -v
```

As travas de segurança valem igual: limite diário, máximo por acionamento e
intervalo mínimo. Comando barrado responde em `aquafeeder/evento` com
`{"erro":"..."}`.

## Opção 3 — VPN / Tailscale (acesso à interface completa)

Se quer o vídeo ao vivo e a interface inteira de fora de casa, instale
**Tailscale** (ou WireGuard) num computador/Raspberry que fique ligado em casa e
entre na rede por VPN. Aí você acessa `http://aquafeeder.local` como se
estivesse em casa.

Muitos roteadores modernos já trazem servidor VPN embutido — vale checar.

## Opção 4 — não faça isso: porta aberta no roteador

Encaminhar a porta 80 do alimentador para a internet expõe um dispositivo
caseiro a varredura constante. Se ainda assim quiser:

- **defina uma senha forte na interface** (sem senha, qualquer um alimenta seus peixes);
- use uma porta externa alta e aleatória;
- nunca exponha a porta 81 (vídeo);
- prefira colocar atrás de um proxy reverso com HTTPS.

## Integrando com outras coisas

A [API HTTP](04-firmware.md#api-http) é aberta dentro da rede. Exemplos de uso:

- **n8n / Home Assistant** — chamada HTTP em `/api/feed?p=1` em qualquer fluxo.
- **Alexa/Google** via rotina do Home Assistant.
- **Script no PC**: `curl -b ck.txt -X POST http://aquafeeder.local/api/feed?p=1`

Um detalhe útil: o `/api/status` devolve `fedToday`, então você pode montar um
alerta externo ("se às 21h ainda não alimentou hoje, me avise").
