"""Main-menu prop set -> Unreal Engine 5 import script (content only, never touches a level).

Unreal: enable "Python Editor Script Plugin" (once, restart), then Tools > Execute Python Script... and pick this file.
Source layout (this file sits next to the model folders):
  Assets/Mustafa/MenuModels/<Name>/SM_<Name>.fbx           one FBX per prop (metres, pivot floor-centre, front +X)
  Assets/Mustafa/MenuModels/<Name>/Textures/*.jpg           Tripo textures: basecolor / normal (OpenGL) / metallic / roughness

Creates (root can be changed with the environment variable MENU_ROOT):
  /Game/ModeLM/Meshes/SM_*                                  static meshes (collision: complex as simple)
  /Game/ModeLM/Textures/<Name>/T_*                          textures (normal map with flipped green = OpenGL source)
  /Game/ModeLM/Materials/M_ModeLM_Tripo + MI_<Name>         one master material, one instance per prop
"""
import os
import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.environ.get("MENU_ROOT", "/Game/ModeLM")
LOGFILE = os.environ.get("MENU_LOG")
ASSETS = unreal.EditorAssetLibrary
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
MEL = unreal.MaterialEditingLibrary
LOG = []


def L(msg):
    LOG.append(str(msg))
    unreal.log("ModeLM: " + str(msg))
    if LOGFILE:
        with open(LOGFILE, "w", encoding="utf-8") as f:
            f.write("\n".join(LOG))


def import_files(tasks_spec):
    tasks = []
    for filename, dest, opts in tasks_spec:
        t = unreal.AssetImportTask()
        t.set_editor_property("filename", filename)
        t.set_editor_property("destination_path", dest)
        t.set_editor_property("automated", True)
        t.set_editor_property("replace_existing", True)
        t.set_editor_property("save", False)
        if opts is not None:
            t.set_editor_property("options", opts)
        tasks.append(t)
    TOOLS.import_asset_tasks(tasks)


def master_material():
    path = ROOT + "/Materials/M_ModeLM_Tripo"
    if ASSETS.does_asset_exist(path):
        return ASSETS.load_asset(path)
    mat = TOOLS.create_asset("M_ModeLM_Tripo", ROOT + "/Materials", unreal.Material, unreal.MaterialFactoryNew())
    spec = [("BaseColor", unreal.MaterialProperty.MP_BASE_COLOR, "RGB", unreal.MaterialSamplerType.SAMPLERTYPE_COLOR),
            ("Normal", unreal.MaterialProperty.MP_NORMAL, "RGB", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL),
            ("Metallic", unreal.MaterialProperty.MP_METALLIC, "R", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR),
            ("Roughness", unreal.MaterialProperty.MP_ROUGHNESS, "R", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)]
    for i, (pname, prop, pin, sampler) in enumerate(spec):
        node = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -400, i * 260)
        node.set_editor_property("parameter_name", pname)
        node.set_editor_property("sampler_type", sampler)
        MEL.connect_material_property(node, pin, prop)
    MEL.recompile_material(mat)
    ASSETS.save_loaded_asset(mat)
    return mat


def main():
    master = master_material()
    names = [d for d in sorted(os.listdir(HERE)) if os.path.isdir(os.path.join(HERE, d, "Textures"))]
    # textures first
    specs = []
    for n in names:
        tdir = os.path.join(HERE, n, "Textures")
        for fn in sorted(os.listdir(tdir)):
            if fn.lower().endswith((".jpg", ".png")):
                specs.append((os.path.join(tdir, fn), "%s/Textures/%s" % (ROOT, n), None))
    import_files(specs)
    # meshes
    specs = []
    for n in names:
        for fn in os.listdir(os.path.join(HERE, n)):
            if fn.lower().endswith(".fbx"):
                o = unreal.FbxImportUI()
                o.set_editor_property("import_mesh", True)
                o.set_editor_property("import_as_skeletal", False)
                o.set_editor_property("import_materials", False)
                o.set_editor_property("import_textures", False)
                o.set_editor_property("import_animations", False)
                specs.append((os.path.join(HERE, n, fn), ROOT + "/Meshes", o))
    import_files(specs)
    # materials + assignment
    for n in names:
        tex_dir = "%s/Textures/%s" % (ROOT, n)
        found = {}
        for p in ASSETS.list_assets(tex_dir, recursive=False):
            t = ASSETS.load_asset(p)
            low = t.get_name().lower()
            for key in ("basecolor", "normal", "metallic", "roughness"):
                if key in low:
                    found[key] = t
        for key, tex in found.items():
            if key == "normal":
                tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
                tex.set_editor_property("flip_green_channel", True)
                tex.set_editor_property("srgb", False)
            elif key in ("metallic", "roughness"):
                tex.set_editor_property("srgb", False)
            ASSETS.save_loaded_asset(tex)
        mi_path = "%s/Materials/MI_%s" % (ROOT, n)
        mi = ASSETS.load_asset(mi_path) if ASSETS.does_asset_exist(mi_path) else \
            TOOLS.create_asset("MI_" + n, ROOT + "/Materials", unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        mi.set_editor_property("parent", master)
        for key, pname in (("basecolor", "BaseColor"), ("normal", "Normal"), ("metallic", "Metallic"), ("roughness", "Roughness")):
            if key in found:
                MEL.set_material_instance_texture_parameter_value(mi, pname, found[key])
            else:
                L("missing %s texture for %s" % (key, n))
        MEL.update_material_instance(mi)
        ASSETS.save_loaded_asset(mi)
        for p in ASSETS.list_assets(ROOT + "/Meshes", recursive=False):
            sm = ASSETS.load_asset(p)
            if isinstance(sm, unreal.StaticMesh) and sm.get_name().lower().endswith(n.lower()) or (isinstance(sm, unreal.StaticMesh) and n.lower() in sm.get_name().lower()):
                mats = list(sm.get_editor_property("static_materials"))
                for slot in mats:
                    slot.set_editor_property("material_interface", mi)
                sm.set_editor_property("static_materials", mats)
                sm.get_editor_property("body_setup").set_editor_property("collision_trace_flag", unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
                ASSETS.save_loaded_asset(sm)
                L("%s -> %s" % (sm.get_name(), mi.get_name()))
    L("done")
    if os.environ.get("MENU_AUTOQUIT") == "1":
        unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")


main()
