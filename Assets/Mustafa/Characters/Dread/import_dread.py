"""The Dread -> Unreal Engine 5 import script.

Unreal: Edit > Plugins > enable "Python Editor Script Plugin" (once, restart editor),
then Tools > Execute Python Script... and pick this file. Keep it next to SK_Dread.fbx and the T_*.png files.

Creates in Content/Dread:
  SK_Dread, SK_Dread_Skeleton       skeletal mesh and skeleton
  Textures/T_Dread*                 textures with correct compression settings
  M_Dread_Master                    master material (BaseColor, ORM, Normal parameters)
  MI_Dread_Body, MI_Dread_Chain     material instances, assigned to the mesh slots
"""
import os
import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = "/Game/Dread"
TEX = ROOT + "/Textures"

ASSETS = unreal.EditorAssetLibrary
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
MEL = unreal.MaterialEditingLibrary
TC = unreal.TextureCompressionSettings
ST = unreal.MaterialSamplerType
MP = unreal.MaterialProperty

# texture file -> (sRGB, compression)
TEXTURES = {
    "T_Dread_BC": (True, TC.TC_DEFAULT),
    "T_Dread_ORM": (False, TC.TC_MASKS),
    "T_Dread_N": (False, TC.TC_NORMALMAP),
    "T_DreadChain_BC": (True, TC.TC_DEFAULT),
    "T_DreadChain_ORM": (False, TC.TC_MASKS),
    "T_DreadChain_N": (False, TC.TC_NORMALMAP),
}
# material slot name (from the FBX) -> texture name prefix
SLOTS = {"M_Dread_Body": "T_Dread", "M_Dread_Chain": "T_DreadChain"}


def import_mesh():
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", os.path.join(HERE, "SK_Dread.fbx"))
    task.set_editor_property("destination_path", ROOT)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", False)
    opt = unreal.FbxImportUI()
    opt.set_editor_property("import_mesh", True)
    opt.set_editor_property("import_as_skeletal", True)
    opt.set_editor_property("import_materials", False)
    opt.set_editor_property("import_textures", False)
    opt.set_editor_property("import_animations", False)
    task.set_editor_property("options", opt)
    TOOLS.import_asset_tasks([task])
    return ASSETS.load_asset(ROOT + "/SK_Dread")


def import_textures():
    tasks = []
    for name in TEXTURES:
        t = unreal.AssetImportTask()
        t.set_editor_property("filename", os.path.join(HERE, name + ".png"))
        t.set_editor_property("destination_path", TEX)
        t.set_editor_property("automated", True)
        t.set_editor_property("replace_existing", True)
        t.set_editor_property("save", False)
        tasks.append(t)
    TOOLS.import_asset_tasks(tasks)
    out = {}
    for name, (srgb, comp) in TEXTURES.items():
        tex = ASSETS.load_asset(TEX + "/" + name)
        tex.set_editor_property("srgb", srgb)
        tex.set_editor_property("compression_settings", comp)
        ASSETS.save_loaded_asset(tex)
        out[name] = tex
    return out


def node(material, kind, x, y, **props):
    e = MEL.create_material_expression(material, kind, x, y)
    for k, v in props.items():
        e.set_editor_property(k, v)
    return e


def build_master(textures):
    path = ROOT + "/M_Dread_Master"
    if ASSETS.does_asset_exist(path):
        return ASSETS.load_asset(path)
    m = TOOLS.create_asset("M_Dread_Master", ROOT, unreal.Material, unreal.MaterialFactoryNew())
    bc = node(m, unreal.MaterialExpressionTextureSampleParameter2D, -600, -300, parameter_name="BaseColor", sampler_type=ST.SAMPLERTYPE_COLOR, texture=textures["T_Dread_BC"])
    orm = node(m, unreal.MaterialExpressionTextureSampleParameter2D, -600, 0, parameter_name="ORM", sampler_type=ST.SAMPLERTYPE_MASKS, texture=textures["T_Dread_ORM"])
    nr = node(m, unreal.MaterialExpressionTextureSampleParameter2D, -600, 300, parameter_name="Normal", sampler_type=ST.SAMPLERTYPE_NORMAL, texture=textures["T_Dread_N"])
    MEL.connect_material_property(bc, "RGB", MP.MP_BASE_COLOR)
    MEL.connect_material_property(orm, "R", MP.MP_AMBIENT_OCCLUSION)
    MEL.connect_material_property(orm, "G", MP.MP_ROUGHNESS)
    MEL.connect_material_property(orm, "B", MP.MP_METALLIC)
    MEL.connect_material_property(nr, "RGB", MP.MP_NORMAL)
    MEL.recompile_material(m)
    ASSETS.save_loaded_asset(m)
    return m


def build_instance(master, textures, prefix, name):
    path = ROOT + "/" + name
    if ASSETS.does_asset_exist(path):
        mi = ASSETS.load_asset(path)
    else:
        mi = TOOLS.create_asset(name, ROOT, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    MEL.set_material_instance_parent(mi, master)
    MEL.set_material_instance_texture_parameter_value(mi, "BaseColor", textures[prefix + "_BC"])
    MEL.set_material_instance_texture_parameter_value(mi, "ORM", textures[prefix + "_ORM"])
    MEL.set_material_instance_texture_parameter_value(mi, "Normal", textures[prefix + "_N"])
    MEL.update_material_instance(mi)
    ASSETS.save_loaded_asset(mi)
    return mi


def main():
    mesh = import_mesh()
    textures = import_textures()
    master = build_master(textures)
    instances = {slot: build_instance(master, textures, prefix, "MI_" + slot[2:]) for slot, prefix in SLOTS.items()}
    mats = list(mesh.get_editor_property("materials"))
    for m in mats:
        slot = str(m.get_editor_property("material_slot_name"))
        if slot in instances:
            m.set_editor_property("material_interface", instances[slot])
        else:
            unreal.log_warning("Dread: unknown material slot " + slot)
    mesh.set_editor_property("materials", mats)
    ASSETS.save_loaded_asset(mesh)
    unreal.log("Dread: imported SK_Dread and assigned materials")


main()
