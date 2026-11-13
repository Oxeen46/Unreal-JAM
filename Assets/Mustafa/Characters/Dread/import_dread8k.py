"""The Dread (7.7k tris, PBRgen materials) -> Unreal Engine 5 import script.

Unreal: Edit > Plugins > enable "Python Editor Script Plugin" (once, restart editor),
then Tools > Execute Python Script... and pick this file. Keep it next to SK_Dread8k.fbx and the Textures folder.

Creates in Content/Dread8k:
  SK_Dread8k, SK_Dread8k_Skeleton   skeletal mesh and skeleton
  Textures/*                        textures with correct compression settings
  M_Dread_Master                    master material: tiled PBR (UV1 x Tiling) + Tripo detail normal (UV0)
  MI_Dread_Skin/Suit/Cloth/Iron     material instances, assigned to the mesh slots by name
"""
import os
import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
TEXDIR = os.path.join(HERE, "Textures")
ROOT = "/Game/Dread8k"
TEX = ROOT + "/Textures"

ASSETS = unreal.EditorAssetLibrary
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
MEL = unreal.MaterialEditingLibrary
TC = unreal.TextureCompressionSettings
ST = unreal.MaterialSamplerType
MP = unreal.MaterialProperty

# slot (material name in the FBX) -> (texture prefix, tiles per metre)
SLOTS = {
    "M_Dread_Skin": ("T_M_Dread_Skin", 2.0),
    "M_Dread_Suit": ("T_M_Dread_Suit", 2.0),
    "M_Dread_Cloth": ("T_M_Dread_Cloth", 2.86),
    "M_Dread_Iron": ("T_M_Dread_Iron", 2.0),
}


def tex_settings():
    t = {}
    for prefix, _ in SLOTS.values():
        t[prefix + "_BC"] = (True, TC.TC_DEFAULT)
        t[prefix + "_ORM"] = (False, TC.TC_MASKS)
        t[prefix + "_N"] = (False, TC.TC_NORMALMAP)
    t["T_Dread8k_DetailN"] = (False, TC.TC_NORMALMAP)
    t["T_Flat_N"] = (False, TC.TC_NORMALMAP)
    return t


def import_mesh():
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", os.path.join(HERE, "SK_Dread8k.fbx"))
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
    return ASSETS.load_asset(ROOT + "/SK_Dread8k")


def import_textures():
    settings = tex_settings()
    tasks = []
    for name in settings:
        t = unreal.AssetImportTask()
        t.set_editor_property("filename", os.path.join(TEXDIR, name + ".png"))
        t.set_editor_property("destination_path", TEX)
        t.set_editor_property("automated", True)
        t.set_editor_property("replace_existing", True)
        t.set_editor_property("save", False)
        tasks.append(t)
    TOOLS.import_asset_tasks(tasks)
    out = {}
    for name, (srgb, comp) in settings.items():
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


def build_master(tex):
    path = ROOT + "/M_Dread_Master"
    if ASSETS.does_asset_exist(path):
        return ASSETS.load_asset(path)
    m = TOOLS.create_asset("M_Dread_Master", ROOT, unreal.Material, unreal.MaterialFactoryNew())
    uv1 = node(m, unreal.MaterialExpressionTextureCoordinate, -1500, 0, coordinate_index=1)
    tiling = node(m, unreal.MaterialExpressionScalarParameter, -1500, 160, parameter_name="Tiling", default_value=2.0)
    scaled = node(m, unreal.MaterialExpressionMultiply, -1300, 60)
    MEL.connect_material_expressions(uv1, "", scaled, "A")
    MEL.connect_material_expressions(tiling, "", scaled, "B")
    bc = node(m, unreal.MaterialExpressionTextureSampleParameter2D, -1000, -300, parameter_name="BaseColor",
              sampler_type=ST.SAMPLERTYPE_COLOR, texture=tex["T_M_Dread_Skin_BC"])
    orm = node(m, unreal.MaterialExpressionTextureSampleParameter2D, -1000, 0, parameter_name="ORM",
               sampler_type=ST.SAMPLERTYPE_MASKS, texture=tex["T_M_Dread_Skin_ORM"])
    nr = node(m, unreal.MaterialExpressionTextureSampleParameter2D, -1000, 300, parameter_name="Normal",
              sampler_type=ST.SAMPLERTYPE_NORMAL, texture=tex["T_M_Dread_Skin_N"])
    for s in (bc, orm, nr):
        MEL.connect_material_expressions(scaled, "", s, "UVs")
    uv0 = node(m, unreal.MaterialExpressionTextureCoordinate, -1000, 600, coordinate_index=0)
    dn = node(m, unreal.MaterialExpressionTextureSampleParameter2D, -700, 600, parameter_name="DetailNormal",
              sampler_type=ST.SAMPLERTYPE_NORMAL, texture=tex["T_Flat_N"])
    MEL.connect_material_expressions(uv0, "", dn, "UVs")
    MEL.connect_material_property(bc, "RGB", MP.MP_BASE_COLOR)
    MEL.connect_material_property(orm, "R", MP.MP_AMBIENT_OCCLUSION)
    MEL.connect_material_property(orm, "G", MP.MP_ROUGHNESS)
    MEL.connect_material_property(orm, "B", MP.MP_METALLIC)
    try:
        fn = ASSETS.load_asset("/Engine/Functions/Engine_MaterialFunctions01/Texturing/BlendAngleCorrectedNormals")
        call = node(m, unreal.MaterialExpressionMaterialFunctionCall, -300, 400, material_function=fn)
        MEL.connect_material_expressions(nr, "RGB", call, "BaseNormal")
        MEL.connect_material_expressions(dn, "RGB", call, "AdditionalNormal")
        MEL.connect_material_property(call, "", MP.MP_NORMAL)
    except Exception as error:
        unreal.log_warning("Dread: detail normal blend skipped: {}".format(error))
        MEL.connect_material_property(nr, "RGB", MP.MP_NORMAL)
    MEL.recompile_material(m)
    ASSETS.save_loaded_asset(m)
    return m


def build_instance(master, tex, slot, prefix, tiles):
    path = ROOT + "/MI_" + slot[2:]
    if ASSETS.does_asset_exist(path):
        mi = ASSETS.load_asset(path)
    else:
        mi = TOOLS.create_asset("MI_" + slot[2:], ROOT, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    MEL.set_material_instance_parent(mi, master)
    MEL.set_material_instance_texture_parameter_value(mi, "BaseColor", tex[prefix + "_BC"])
    MEL.set_material_instance_texture_parameter_value(mi, "ORM", tex[prefix + "_ORM"])
    MEL.set_material_instance_texture_parameter_value(mi, "Normal", tex[prefix + "_N"])
    MEL.set_material_instance_texture_parameter_value(mi, "DetailNormal",
                                                      tex["T_Flat_N"] if slot == "M_Dread_Iron" else tex["T_Dread8k_DetailN"])
    MEL.set_material_instance_scalar_parameter_value(mi, "Tiling", tiles)
    MEL.update_material_instance(mi)
    ASSETS.save_loaded_asset(mi)
    return mi


def main():
    mesh = import_mesh()
    tex = import_textures()
    master = build_master(tex)
    inst = {slot: build_instance(master, tex, slot, prefix, tiles) for slot, (prefix, tiles) in SLOTS.items()}
    mats = list(mesh.get_editor_property("materials"))
    for m in mats:
        slot = str(m.get_editor_property("material_slot_name"))
        if slot in inst:
            m.set_editor_property("material_interface", inst[slot])
        else:
            unreal.log_warning("Dread: unknown material slot " + slot)
    mesh.set_editor_property("materials", mats)
    ASSETS.save_loaded_asset(mesh)
    unreal.log("Dread8k: imported SK_Dread8k and assigned materials")


main()
