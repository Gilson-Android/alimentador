# Binários já compilados

Gerados com PlatformIO (espressif32 6.9.x / Arduino core 2.0.17) a partir do
código em `../src`. Servem para gravar sem instalar toolchain nenhum.

| Pasta | Placa |
|---|---|
| `freenove_s3_cam` | Freenove ESP32-S3-WROOM CAM (8 MB flash) — padrão |
| `xiao_s3_sense`   | Seeed XIAO ESP32S3 Sense |
| `s3_sem_camera`   | ESP32-S3 genérico, sem câmera |

O caminho recomendado continua sendo `pio run -t upload`
(ver [docs/04-firmware.md](../../docs/04-firmware.md)). Para gravar direto com esptool:

```bash
pip install esptool
cd freenove_s3_cam
esptool.py --chip esp32s3 --port COM5 --baud 921600 write_flash -z 0x0 bootloader.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 firmware.bin
```

Troque `COM5` pela porta da sua placa. Se der erro de conexão, ponha a placa em
modo download: segure BOOT, toque RESET, solte BOOT.

Depois de gravado, a configuração é toda pela interface web (modo AP no primeiro
boot: rede `AquaFeeder-XXXX`, senha `aquafeeder`, http://192.168.4.1).
