"""Joe's house / neighbourhood PBRgen materials -> Unreal Engine 5 import script (content only, never touches a level).

Unreal: enable "Python Editor Script Plugin" (once, restart), then Tools > Execute Python Script... and pick this file.
Expected layout (this file sits next to the Textures folder):
  Assets/Mustafa/Textures/<M_Joe_...>/import_pbrgen.py + 4 JPEG (BC, N DirectX, ORM, H)

Creates (root can be changed with the environment variable JOE_PBR_ROOT):
  /Game/ModeLM/PBRgen/M_PBRgen                              master material (shared)
  /Game/ModeLM/PBRgen/<M_Joe_...>/MI_<M_Joe_...>   material instances, Tiling set per material
"""
import os
import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
TEX_DIR = os.path.join(HERE, "Textures")
PBR_ROOT = os.environ.get("JOE_PBR_ROOT", "/Game/ModeLM/PBRgen")
LOGFILE = os.environ.get("JOE_PBR_LOG")
TILING = {"M_Joe_Wall_White": 0.5, "M_Joe_Wall_Wallpaper": 1.0, "M_Joe_Ceiling_White": 0.5, "M_Joe_Wall_Tile": 1.0, "M_Joe_Facade_Brick": 0.5, "M_Joe_Basement_Concrete": 0.5}   # tiles per metre
ASSETS = unreal.EditorAssetLibrary
MEL = unreal.MaterialEditingLibrary
LOG = []


def L(msg):
    LOG.append(str(msg))
    unreal.log("JoeMaterials: " + str(msg))
    if LOGFILE:
        with open(LOGFILE, "w", encoding="utf-8") as f:
            f.write("\n".join(LOG))


def main():
    for name, tiling in TILING.items():
        script = os.path.join(TEX_DIR, name, "import_pbrgen.py")
        if not os.path.exists(script):
            L("MISSING PBRgen script " + script)
            continue
        try:
            with open(script, encoding="utf-8") as fh:
                text = fh.read().lstrip("﻿").replace('ROOT = "/Game/PBRgen"', 'ROOT = "%s"' % PBR_ROOT)
            exec(compile(text, script, "exec"), {"__file__": script, "__name__": "__main__"})
        except Exception as e:
            L("PBRgen script failed for %s: %s" % (name, e))
            continue
        mi = ASSETS.load_asset("%s/%s/MI_%s" % (PBR_ROOT, name, name))
        if mi is None:
            L("NO MI " + name)
            continue
        MEL.set_material_instance_scalar_parameter_value(mi, "Tiling", tiling)
        try:
            MEL.set_material_instance_scalar_parameter_value(mi, "DisplacementAmount", 0.0)
        except Exception:
            pass
        MEL.update_material_instance(mi)
        ASSETS.save_loaded_asset(mi)
        L("ready: " + name)
    L("done")
    if os.environ.get("JOE_PBR_AUTOQUIT") == "1":
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")


main()
