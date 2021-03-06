# Name of your project, will set the name of your ROM.
PROJECT_NAME := Wyrmhole
# Run "rgbfix --mbc-type help" for possible MBC types
MBC     := ROM
# Target should be a combination of DMG, CGB and SGB
TARGETS := DMG

include gbsdk/rules.mk

# Game fits in 32k, so put everything in ROM0
LDFLAGS += --tiny

FIXFLAGS += --rom-version 2
FIXFLAGS += --new-licensee HB

tools/gbcompress/gbcompress:
	@echo Building gbcompress
	@mkdir -p $(dir $@)
	@$(MAKE) -C tools/gbcompress

# Convert tilemaps (ALSO INVERTS THEM)
$(BUILD)/assets/%.gbcompress $(BUILD)/assets/%.tilemap: tilemaps/%.png
	@echo Converting $<
	@mkdir -p $(dir $@)
	$(Q)rgbgfx $< -u -o $(BUILD)/assets/$*.2bpp -t $(BUILD)/assets/$*.tilemap
	python tools/InvertColours.py $(BUILD)/assets/$*.2bpp
	@./tools/gbcompress/gbcompress $(BUILD)/assets/$*.2bpp $@

# Convert compressed assets
$(BUILD)/assets/%.gbcompress: assets/%.c.png tools/gbcompress/gbcompress
	@echo Converting $<
	@mkdir -p $(dir $@)
	$(Q)rgbgfx $< -o $(BUILD)/assets/$*.2bpp
	@./tools/gbcompress/gbcompress $(BUILD)/assets/$*.2bpp $@

$(BUILD)/assets/%.gbcompress: assets/%.oam.c.png tools/gbcompress/gbcompress
	@echo Converting $<
	@mkdir -p $(dir $@)
	$(Q)rgbgfx -h $< -o $(BUILD)/assets/$*.2bpp
	@./tools/gbcompress/gbcompress $(BUILD)/assets/$*.2bpp $@

# Add this new clean as a prerequisite for the GBSDK clean.
clean: clean2
clean2:
	@$(MAKE) -C tools/gbcompress clean
