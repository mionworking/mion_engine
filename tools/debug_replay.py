#!/usr/bin/env python3
"""
Utilitário de desenvolvimento para:
- Reconfigurar/compilar o projeto
- Restaurar o snapshot de debug do dungeon por cima do save principal
- Iniciar o jogo já nesse estado (útil para reproduzir bugs como o level-up travando)
"""

import subprocess
import shutil
from pathlib import Path


def run(cmd, cwd=None):
    print(f"$ {' '.join(cmd)}")
    subprocess.check_call(cmd, cwd=cwd)


def main():
    # Paths base — ajuste aqui se seu layout mudar
    repo_root = Path("/home/danten/Documents/G_v2/mion_engine_cpp")
    build_dir = repo_root / "build"
    engine_bin = build_dir / "mion_engine"

    save_dir = Path.home() / ".local" / "share" / "mion" / "mion_engine"
    main_save = save_dir / "save.txt"
    debug_save = save_dir / "debug_dungeon_autosave.txt"
    backup_save = save_dir / "save_backup.txt"

    # 1) Gera/atualiza build
    build_dir.mkdir(exist_ok=True)
    run(["cmake", ".."], cwd=build_dir)
    run(["cmake", "--build", ".", "-j8"], cwd=build_dir)

    # 2) Garante diretório de saves
    save_dir.mkdir(parents=True, exist_ok=True)

    # 3) Backup do save atual, se existir
    if main_save.exists():
        print(f"Backing up {main_save} -> {backup_save}")
        shutil.copy2(main_save, backup_save)
    else:
        print(f"Nenhum save principal encontrado em {main_save} (ok).")

    # 4) Restaura snapshot de debug por cima do save principal
    if not debug_save.exists():
        raise SystemExit(f"Snapshot de debug não encontrado: {debug_save}")
    print(f"Restoring debug snapshot {debug_save} -> {main_save}")
    shutil.copy2(debug_save, main_save)

    # 5) Executa o jogo
    if not engine_bin.exists():
        raise SystemExit(f"Binário não encontrado: {engine_bin}")
    run([str(engine_bin)], cwd=repo_root)


if __name__ == "__main__":
    main()

