# O Último Pudim — fatia vertical para N64 (libdragon)

Reescrita em C do tutorial/fase 1 do jogo em Godot, agora com os sprites do jogo original
(extraídos do `.pck` e reduzidos para a escala do N64).

## Compilar
Com a libdragon instalada (https://github.com/DragonMinded/libdragon), na pasta do projeto:

    libdragon make          # via CLI/Docker da libdragon
    # ou: make              # com a toolchain instalada e N64_INST definido

O Makefile converte os PNGs de `assets/` em `.sprite` (RGBA16) com o `mksprite` da libdragon
e os empacota no ROM. Gera `pudim.z64`. Rode em um emulador (ares ou simple64) ou num flashcart.

## Controles
| N64                | Ação                                              |
|--------------------|---------------------------------------------------|
| Analógico / D-pad  | andar (analógico ↑/↓ mira em diagonal)            |
| A                  | pular (pulo duplo)                                |
| B (segurar)        | atirar                                            |
| Z ou R             | dash (gasta 20 de stamina, precisa de direção)    |
| C-esquerda/direita | trocar de arma (só as que você tem)               |
| C-cima             | comprar arma perto da loja azul (col. 8)          |
| Start              | reiniciar depois de morrer ou vencer              |

## O que veio do jogo original
Pulo duplo, dash com stamina (100, +0,3/frame), vida 100, 3 armas (Pistola 10 de dano,
Rifle 7 mais rápido, Shotgun 3 balas de 15), pirata patrulheiro que vira na parede e na
borda, moedas, loja, dinheiro e armas que persistem ao morrer. Os valores foram escalados
de 1280×720 para 320×240; custos e vida do pirata foram ajustados para a demo.

## Assets (pasta `assets/`)
27 PNGs, todos ≤ 32×32 para caber no TMEM (4 KB) do RDP: player (idle 4, walk 6, jump 2),
pirata (walk 6), moeda, 3 armas, bala, tile de madeira, placa da loja, altar e pudim.
Os frames foram recortados das spritesheets do jogo, reduzidos com Lanczos e o alfa foi
binarizado (RGBA16 só tem 1 bit de alfa). Os nomes que o código carrega estão em `load_assets()`.
Se o `mksprite` da sua versão não aceitar `--format RGBA16`, ajuste `MKSPRITE_FLAGS` no Makefile.

## Limitações
- Sem sons, música, bosses, fases 2 e 3, nem jet-pack/papagaio/barril.
- Lógica a 60 FPS fixos: em consoles PAL (50 Hz) o jogo roda mais lento.
- Não foi compilado com a toolchain MIPS nem rodado em emulador. Testei em PC com um stub da
  libdragon (`test/`): compila sem avisos, carrega os 27 assets, roda 3600 frames sem crash, e
  um bot (`test/bot.c`) completa a fase.
- Os sprites são do jogo original: use com a permissão de quem os criou.

## Próximos passos
1. Áudio: converter para WAV64 (`audioconv64`) e tocar com `wav64_play`.
2. Portar a fase 2 (Capitão) e a fase 3 (Ktulu), reaproveitando `body_move` e `Enemy`.
3. Mais animações (dash, ataque do pirata) e fundo com parallax.
