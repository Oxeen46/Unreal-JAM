"""ProseApart modular apartment kit -> Unreal Engine 5 import script (content only, never touches a level).

Unreal: enable "Python Editor Script Plugin" (once, restart), then Tools > Execute Python Script... and pick this file.
Expected layout (this file sits next to the kit):
  Assets/Mustafa/ProseApart/FBX/SM_*.fbx                41 single-part FBX (pivot at the part corner), material slots = M_Wall_Interior, M_Floor, ...
  Assets/Mustafa/ProseApart_Kit.fbx                     original one-sheet export (kept, no longer imported)
  Assets/Mustafa/Textures/<M_...>/import_pbrgen.py      10 PBRgen materials (JPEG textures)

Creates (roots can be changed with the environment variables PROSE_ROOT / PROSE_PBR_ROOT):
  /Game/Developers/mustafa/ProseApart/Meshes/SM_*...    one static mesh per kit part (units cm), materials assigned by slot name
  /Game/Developers/mustafa/PBRgen/<M_...>/MI_<M_...>    PBRgen material instances, Tiling set per material
"""
import os
import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
PARTS_DIR = os.path.join(HERE, "ProseApart", "FBX")      # 41 single-part FBX (pivot = part corner)
TEX_DIR = os.path.join(HERE, "Textures")
ROOT = os.environ.get("PROSE_ROOT", "/Game/Developers/mustafa/ProseApart")
PBR_ROOT = os.environ.get("PROSE_PBR_ROOT", "/Game/Developers/mustafa/PBRgen")
MESH_DIR = ROOT + "/Meshes"
LOGFILE = os.environ.get("PROSE_LOG")

# tiles per metre per material (values proven in the earlier test)
TILING = {"M_Wall_Exterior": 0.5, "M_Wall_Interior": 0.5, "M_Floor": 1.0, "M_Roof": 0.5, "M_Ceiling": 0.5,
          "M_Concrete": 0.5, "M_Door": 0.5, "M_Frame": 1.0, "M_Railing": 2.0, "M_Window_Glass": 0.5}

ASSETS = unreal.EditorAssetLibrary
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
MEL = unreal.MaterialEditingLibrary
LOG = []


def L(msg):
    LOG.append(str(msg))
    unreal.log("ProseApart: " + str(msg))
    if LOGFILE:
        try:
            with open(LOGFILE, "w", encoding="utf-8") as f:
                f.write("\n".join(LOG))
        except Exception:
            pass


def build_materials():
    out = {}
    for name, tiling in TILING.items():
        script = os.path.join(TEX_DIR, name, "import_pbrgen.py")
        if not os.path.exists(script):
            L("MISSING PBRgen script " + script)
            continue
        try:
            with open(script, encoding="utf-8") as fh:
                text = fh.read().lstrip("﻿ï»¿").replace('ROOT = "/Game/PBRgen"', 'ROOT = "%s"' % PBR_ROOT)
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
        out[name] = mi
    return out


def import_kit():
    """one FBX per part (ProseApart/FBX): every part keeps its own pivot (its corner) and correct units."""
    tasks, names = [], []
    for fn in sorted(os.listdir(PARTS_DIR)):
        if not fn.lower().endswith(".fbx"):
            continue
        t = unreal.AssetImportTask()
        t.set_editor_property("filename", os.path.join(PARTS_DIR, fn))
        t.set_editor_property("destination_path", MESH_DIR)
        t.set_editor_property("automated", True)
        t.set_editor_property("replace_existing", True)
        t.set_editor_property("save", False)
        opt = unreal.FbxImportUI()
        opt.set_editor_property("import_mesh", True)
        opt.set_editor_property("import_as_skeletal", False)
        opt.set_editor_property("import_materials", False)
        opt.set_editor_property("import_textures", False)
        opt.set_editor_property("import_animations", False)
        t.set_editor_property("options", opt)
        tasks.append(t)
        names.append(os.path.splitext(fn)[0])
    TOOLS.import_asset_tasks(tasks)
    return names

def assign(names, mapping):
    unmapped = set()
    done = 0
    for n in names:
        sm = ASSETS.load_asset(MESH_DIR + "/" + n)
        if sm is None:
            continue
        mats = list(sm.get_editor_property("static_materials"))
        for slot in mats:
            sname = str(slot.get_editor_property("material_slot_name"))
            if sname in mapping:
                slot.set_editor_property("material_interface", mapping[sname])
            else:
                unmapped.add(sname)
        sm.set_editor_property("static_materials", mats)
        ASSETS.save_loaded_asset(sm)
        done += 1
    if unmapped:
        L("unmapped slots: %s" % sorted(unmapped))
    return done


def main():
    mats = build_materials()
    L("materials ready: %d / %d" % (len(mats), len(TILING)))
    names = import_kit()
    L("meshes imported: %d" % len(names))
    L("materials assigned on %d meshes" % assign(names, mats))
    L("done")
    if os.environ.get("PROSE_AUTOQUIT") == "1":
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")


main()
