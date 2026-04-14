BUILD_DIR := build
BINARY    := $(BUILD_DIR)/mion_engine
PYTHON    ?= python3

.PHONY: all configure build run clean test stress render_stress sprite_bench gen-placeholders preview-placeholders verify-placeholders gen-placeholders-backlog preview-placeholders-backlog

all: build

configure:
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug

build: configure
	cmake --build $(BUILD_DIR) -j$(shell nproc)

run: build
	./$(BINARY)

test: build
	cd $(BUILD_DIR) && ctest --output-on-failure

STRESS_ENEMIES ?= 180
STRESS_FRAMES  ?= 600

stress: build
	./$(BUILD_DIR)/mion_stress $(STRESS_ENEMIES) $(STRESS_FRAMES)

# Render stress: janela real SDL, mede FPS por faixa de inimigos
# Parâmetros: largura altura max_inimigos passo frames_por_etapa
RS_WIDTH        ?= 1280
RS_HEIGHT       ?= 720
RS_MAX          ?= 500
RS_STEP         ?= 50
RS_FRAMES       ?= 300
RS_FRAMES_MULT  ?= 1

render_stress: build
	./$(BUILD_DIR)/mion_render_stress $(RS_WIDTH) $(RS_HEIGHT) $(RS_MAX) $(RS_STEP) $(RS_FRAMES) $(RS_FRAMES_MULT)

# Sprite bench: textura sintética, compara impacto de frames/anim
# Rode 3x com SB_FRAMES_PER_ANIM=4, 8, 16 para comparar
SB_WIDTH          ?= 1280
SB_HEIGHT         ?= 720
SB_MAX_ACTORS     ?= 500
SB_STEP_ACTORS    ?= 50
SB_FRAMES_PER_ANIM?= 4
SB_RENDER_FRAMES  ?= 300

sprite_bench: build
	./$(BUILD_DIR)/mion_sprite_bench $(SB_WIDTH) $(SB_HEIGHT) $(SB_MAX_ACTORS) $(SB_STEP_ACTORS) $(SB_FRAMES_PER_ANIM) $(SB_RENDER_FRAMES)

gen-placeholders:
	$(PYTHON) tools/gen_placeholder_textures.py

preview-placeholders:
	$(PYTHON) tools/preview_placeholders.py

gen-placeholders-backlog:
	$(PYTHON) tools/gen_placeholder_textures.py --manifest tools/texture_backlog_contract.json

preview-placeholders-backlog:
	$(PYTHON) tools/preview_placeholders.py --manifest tools/texture_backlog_contract.json --output assets/placeholders/preview_backlog.png

verify-placeholders: gen-placeholders preview-placeholders
	MION_TEXTURE_INTEGRATION_TESTS=1 ./$(BUILD_DIR)/mion_tests_v2

clean:
	rm -rf $(BUILD_DIR)

# Build release (otimizado, menor binário)
release:
	cmake -B build_release -DCMAKE_BUILD_TYPE=Release
	cmake --build build_release -j$(shell nproc)
