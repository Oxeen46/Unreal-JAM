import os
import unreal

PBRGEN = {"name":"M_Concrete","files":{"BaseColor":"T_M_Concrete_BC.jpg","Normal":"T_M_Concrete_N.jpg","ORM":"T_M_Concrete_ORM.jpg","Height":"T_M_Concrete_H.jpg"},"displacement":0.3891565421106294,"zero":0.5}

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = "/Game/PBRgen"
MASTER = ROOT + "/M_PBRgen"
MAX_DISPLACEMENT = 50.0

MEL = unreal.MaterialEditingLibrary
ASSETS = unreal.EditorAssetLibrary
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
TC = unreal.TextureCompressionSettings
ST = unreal.MaterialSamplerType
MP = unreal.MaterialProperty

SLOTS = {
    "BaseColor": (True, TC.TC_DEFAULT, ST.SAMPLERTYPE_COLOR),
    "Normal": (False, TC.TC_NORMALMAP, ST.SAMPLERTYPE_NORMAL),
    "ORM": (False, TC.TC_MASKS, ST.SAMPLERTYPE_MASKS),
    "Height": (False, TC.TC_GRAYSCALE, ST.SAMPLERTYPE_LINEAR_GRAYSCALE),
}


def import_textures(folder):
    tasks = []
    for slot in SLOTS:
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", os.path.join(HERE, PBRGEN["files"][slot]))
        task.set_editor_property("destination_path", folder)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", False)
        tasks.append(task)
    TOOLS.import_asset_tasks(tasks)

    textures = {}
    for slot, (srgb, compression, _) in SLOTS.items():
        name = os.path.splitext(PBRGEN["files"][slot])[0]
        texture = ASSETS.load_asset(folder + "/" + name)
        texture.set_editor_property("srgb", srgb)
        texture.set_editor_property("compression_settings", compression)
        ASSETS.save_loaded_asset(texture)
        textures[slot] = texture
    return textures


def node(material, kind, x, y, **properties):
    expression = MEL.create_material_expression(material, kind, x, y)
    for key, value in properties.items():
        expression.set_editor_property(key, value)
    return expression


def scalar(material, name, value, x, y):
    return node(material, unreal.MaterialExpressionScalarParameter, x, y, parameter_name=name, default_value=value)


def link(source, output, target, input_name):
    if not MEL.connect_material_expressions(source, output, target, input_name):
        unreal.log_warning("PBRgen: could not connect to " + input_name)


def build_master(defaults):
    m = TOOLS.create_asset("M_PBRgen", ROOT, unreal.Material, unreal.MaterialFactoryNew())
    m.set_editor_property("use_material_attributes", True)
    m.set_editor_property("used_with_nanite", True)
    out = node(m, unreal.MaterialExpressionMakeMaterialAttributes, -250, -300)
    MEL.connect_material_property(out, "", MP.MP_MATERIAL_ATTRIBUTES)

    uv = node(m, unreal.MaterialExpressionTextureCoordinate, -1900, 0)
    tiling = scalar(m, "Tiling", 1.0, -1900, 150)
    scaled = node(m, unreal.MaterialExpressionMultiply, -1700, 50)
    link(uv, "", scaled, "A")
    link(tiling, "", scaled, "B")

    samplers = {}
    for i, (slot, (_, _, sampler)) in enumerate(SLOTS.items()):
        samplers[slot] = node(m, unreal.MaterialExpressionTextureSampleParameter2D, -1400, i * 320 - 400,
                              parameter_name=slot, texture=defaults[slot], sampler_type=sampler)
        link(scaled, "", samplers[slot], "UVs")

    link(samplers["BaseColor"], "RGB", out, "BaseColor")

    strength = scalar(m, "NormalStrength", 1.0, -1400, -700)
    xy = node(m, unreal.MaterialExpressionAppendVector, -1150, -700)
    link(strength, "", xy, "A")
    link(strength, "", xy, "B")
    one = node(m, unreal.MaterialExpressionConstant, -1150, -560, r=1.0)
    xyz = node(m, unreal.MaterialExpressionAppendVector, -950, -650)
    link(xy, "", xyz, "A")
    link(one, "", xyz, "B")
    normal = node(m, unreal.MaterialExpressionMultiply, -750, -300)
    link(samplers["Normal"], "RGB", normal, "A")
    link(xyz, "", normal, "B")
    link(normal, "", out, "Normal")

    link(samplers["ORM"], "R", out, "AmbientOcclusion")
    link(samplers["ORM"], "B", out, "Metallic")
    rough_tweak = scalar(m, "RoughnessMultiplier", 1.0, -1050, 250)
    rough = node(m, unreal.MaterialExpressionMultiply, -750, 150)
    link(samplers["ORM"], "G", rough, "A")
    link(rough_tweak, "", rough, "B")
    link(rough, "", out, "Roughness")

    try:
        m.set_editor_property("enable_tessellation", True)
        scaling = m.get_editor_property("displacement_scaling")
        scaling.set_editor_property("magnitude", MAX_DISPLACEMENT)
        scaling.set_editor_property("center", 0.5)
        m.set_editor_property("displacement_scaling", scaling)

        zero = scalar(m, "DisplacementZero", 0.5, -1050, 600)
        amount = scalar(m, "DisplacementAmount", 1.0, -1050, 720)
        centered = node(m, unreal.MaterialExpressionSubtract, -850, 520)
        link(samplers["Height"], "R", centered, "A")
        link(zero, "", centered, "B")
        sized = node(m, unreal.MaterialExpressionMultiply, -700, 600)
        link(centered, "", sized, "A")
        link(amount, "", sized, "B")
        fraction = node(m, unreal.MaterialExpressionMultiply, -550, 600, const_b=1.0 / MAX_DISPLACEMENT)
        link(sized, "", fraction, "A")
        offset = node(m, unreal.MaterialExpressionAdd, -400, 600, const_b=0.5)
        link(fraction, "", offset, "A")
        link(offset, "", out, "Displacement")
    except Exception as error:
        unreal.log_warning("PBRgen: displacement was not connected: {}".format(error))

    MEL.recompile_material(m)
    ASSETS.save_loaded_asset(m)
    return m


def main():
    name = PBRGEN["name"]
    folder = ROOT + "/" + name
    if ASSETS.does_asset_exist(MASTER):
        master = ASSETS.load_asset(MASTER)
    else:
        master = build_master(import_textures(ROOT + "/Defaults"))

    textures = import_textures(folder)
    instance_path = folder + "/MI_" + name
    if ASSETS.does_asset_exist(instance_path):
        instance = ASSETS.load_asset(instance_path)
    else:
        instance = TOOLS.create_asset("MI_" + name, folder, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())

    MEL.set_material_instance_parent(instance, master)
    for slot, texture in textures.items():
        MEL.set_material_instance_texture_parameter_value(instance, slot, texture)
    MEL.set_material_instance_scalar_parameter_value(instance, "DisplacementAmount", PBRGEN["displacement"])
    MEL.set_material_instance_scalar_parameter_value(instance, "DisplacementZero", PBRGEN["zero"])
    MEL.update_material_instance(instance)
    ASSETS.save_loaded_asset(instance)
    ASSETS.sync_browser_to_objects([instance_path])
    unreal.log("PBRgen: created " + instance_path)


main()
