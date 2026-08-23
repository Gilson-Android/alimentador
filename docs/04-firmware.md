# 4. Firmware

## Gravar (PlatformIO — recomendado)

Instale o [VS Code](https://code.visualstudio.com) + extensão **PlatformIO IDE**,
abra a pasta `firmware/` e clique em *Upload*. Ou por linha de comando:

```bash
pip install --user platformio
cd firmware
pio run -e freenove_s3_cam -t upload
pio device monitor
```

Ambientes disponíveis (o `-e`):

| Ambiente | Placa |
|---|---|
| `freenove_s3_cam` | Freenove ESP32-S3-WROOM CAM (padrão) |
| `xiao_s3_sense` | Seeed XIAO ESP32S3 Sense |
| `s3_sem_camera` | qualquer ESP32-S3 sem câmera |

**Placa Freenove:** grave pela porta **UART** (a que tem o chip USB-serial), não
pela USB nativa. É onde o log serial aparece também. Se a gravação falhar,
segure BOOT, dê um toque em RESET, solte BOOT e tente de novo.

**Trocar de pinos?** Está tudo em `firmware/include/config.h`.

## Primeiro boot

Não há Wi-Fi salvo, então o aparelho sobe em **modo configuração**:

1. Procure a rede Wi-Fi **`AquaFeeder-XXXX`** e conecte com a senha `aquafeeder`.
2. Abra **http://192.168.4.1**.
3. Vá em *Ajustes → Rede e sistema*, preencha o Wi-Fi da sua casa, **defina uma
   senha para a interface** e salve. O aparelho reinicia e entra na sua rede.
4. Depois disso, acesse por **http://aquafeeder.local** (ou pelo IP que aparece
   no monitor serial / na lista de dispositivos do seu roteador).

Vale **reservar o IP** (DHCP estático) no roteador — assim o endereço nunca muda.

## A interface

Quatro abas:

- **Agora** — porções do dia, última alimentação, botões de 1/2/3 porções, foto
  e vídeo da câmera, e o controle de manutenção da rosca (avançar/recuar).
- **Agenda** — 8 horários. Cada linha: liga/desliga, hora, porções e os dias da
  semana (D S T Q Q S S). Não esqueça de *Salvar agenda*.
- **Ajustes** — dosagem, travas de segurança, câmera, Telegram, rede.
  Em *Câmera* existe o campo **URL de câmera externa**: se você não tem um módulo
  OV2640, cole ali o endereço de qualquer fonte MJPEG/JPEG (celular velho com o app
  *IP Webcam*, câmera Wi-Fi) e a aba *Agora* passa a mostrar essa imagem. O
  navegador busca a URL direto; o ESP32 não faz proxy e não gasta memória com isso.
- **Histórico** — as últimas 24 alimentações, com origem (agenda, manual,
  telegram, botão) e quantos grãos o sensor contou.

## Atualizações depois da primeira gravação

Funciona OTA pela rede:

```bash
pio run -e freenove_s3_cam -t upload --upload-port aquafeeder.local
```

A senha do OTA é a mesma senha da interface.

## API HTTP

Tudo que a interface faz está exposto — dá para automatizar (n8n, Home
Assistant, cron, script). Autenticação por cookie de sessão:

```bash
# login (guarda o cookie)
curl -c ck.txt -X POST http://aquafeeder.local/api/login \
     -d '{"pass":"SUA_SENHA"}'

# estado
curl -b ck.txt http://aquafeeder.local/api/status

# alimentar 2 porções
curl -b ck.txt -X POST "http://aquafeeder.local/api/feed?p=2"

# histórico
curl -b ck.txt http://aquafeeder.local/api/log

# foto
curl -b ck.txt http://aquafeeder.local/snapshot.jpg -o aquario.jpg
```

| Método | Rota | O que faz |
|---|---|---|
| GET | `/` | interface |
| POST | `/api/login` | `{"pass":"..."}` → cookie de sessão |
| GET | `/api/status` | estado geral |
| GET/POST | `/api/config` | lê / grava configuração e agenda |
| POST | `/api/feed?p=N` | alimenta N porções (respeita as travas) |
| POST | `/api/jog?s=N` | gira N meio-passos (negativo = recua) |
| GET | `/api/log` | últimas alimentações |
| POST | `/api/reboot` | reinicia |
| GET | `/snapshot.jpg` | foto JPEG |
| GET | `:81/stream` | vídeo MJPEG (porta 81) |

Erros vêm como `{"err":"mensagem"}` com HTTP 400/401/429.

## Como a agenda decide alimentar

A cada 15 s o firmware avalia cada horário ativo:

```
disparar se:  já passou da hora  E
              a última alimentação foi ANTES dessa hora  E
              não passou mais que "janela de recuperação" minutos
```

Consequências práticas (todas desejáveis):

- Faltou luz às 08:00 e voltou às 08:40 → **alimenta ao ligar** (dentro da janela de 90 min).
- Faltou luz às 08:00 e voltou às 14:00 → **não alimenta**, ficou fora da janela.
- Você alimentou na mão às 07:50 → o horário das 08:00 **é pulado** (já comeram).
- Reset/reboot no meio do dia → não repete refeições já feitas.

O contador do dia e a hora da última refeição ficam gravados na memória
permanente (NVS), então sobrevivem a falta de energia.
