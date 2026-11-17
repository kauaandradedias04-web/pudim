BUILD_DIR = build
include $(N64_INST)/include/n64.mk

# PNGs em assets/ viram .sprite em filesystem/ (RGBA16 = 16 bits, cabe no TMEM de 4 KB)
MKSPRITE_FLAGS = --format RGBA16

src = src/main.c
assets_png  = $(wildcard assets/*.png)
assets_conv = $(patsubst assets/%.png,filesystem/%.sprite,$(assets_png))

all: pudim.z64

$(BUILD_DIR)/pudim.dfs: $(assets_conv)
$(BUILD_DIR)/pudim.elf: $(src:%.c=$(BUILD_DIR)/%.o)

pudim.z64: N64_ROM_TITLE = "O Ultimo Pudim"
pudim.z64: $(BUILD_DIR)/pudim.dfs

clean:
	rm -rf $(BUILD_DIR) filesystem pudim.z64

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean
