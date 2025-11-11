# Firmware voor een RP2040 project in C

## Componenten

### Hoofdcomponenten
- **RP2040 ontwikkelboard** (bijv. Raspberry Pi Pico W)
- **Digilent Blue OLED Display** 128 × 32 pixels (SPI Interface)
- **WS2812 LED module**
- **Rotary encoder** (PEC11R-4220K-S0024)
- **TSDP34138** (Infrarood ontvanger)
- **Potentiometer** 10k (voor volume regeling)

### Voeding
- **L7805ABV** spanningsregelaar (5V)
- **Stekkernetvoeding** (instelbaar)
- **DC BARREL JACK** aansluiting

### Ondersteunende Componenten
- **47μF 16V condensator** -voeding
- **Weerstanden:**
  - 47Ω 25W -WS2812 LED module
  - 10kΩ 25W -PEC11R-4220K-S0024

### Optioneel/Ondersteunend
- **RP2040 Debugger** (voor ontwikkeling)
- **ESP12F** WiFi module//niet gebruikt

## Beschrijving
Deze firmware stuurt een zogenaamde geluidsversterker aan.

## Project Doel
Het idee van dit project is om te leren en te onderzoeken.
Hierom is er hier sprake van experimentele code.

## Compatibiliteit
Het project is gebouwd voor de Pico W maar werkt ook op andere RP2040 bordjes.

## Ondersteuning
Dit project wordt niet meer ondersteund.

## Waarschuwing
**Gebruik op eigen risico.**
