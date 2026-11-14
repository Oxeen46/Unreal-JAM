"""Interior props (Quarantino house, office, kitchen, bedroom...) -> Unreal Engine 5 import script.

Unreal: Edit > Plugins > enable "Python Editor Script Plugin" (once, restart editor),
then Tools > Execute Python Script... and pick this file. Keep this folder structure:

  Assets/Mustafa/Props/Interior/import_props.py      (this file)
  Assets/Mustafa/Props/Interior/FBX/*.fbx            (static meshes, interactive parts)
  Assets/Mustafa/Props/Interior/assembly.json        (rest-pose transforms of the interactive part sets)
  Assets/Mustafa/Textures/<M_Apt_...>/import_pbrgen.py (4 PBRgen materials, JPEG textures)

Creates in Content/Interior:
  Meshes/SM_*                       static meshes (units: cm, front = +X)
  Materials/M_Prop_Basic            simple master (BaseColor, Roughness, Metallic) + MI_* instances for the plain slots
  Materials/M_TV_Screen, MI_TV_Screen  emissive CRT screen (Power, Picture, StaticAmount, ... see below)
and (via the PBRgen scripts) Content/PBRgen/<name>/MI_<name> for the 4 PBRgen materials, with Tiling set per material.

Interactive part sets (Fridge, KitchenCounter, Wardrobe, Nightstand): every part is modelled CLOSED.
  Doors: pivot = hinge (rotate around Z). Drawers: pivot = front-face centre (slide along +X).
  assembly.json holds the rest-pose offset of each part relative to its set root (Blender metres).
  Unreal conversion: x*100, y*-100, z*100, yaw = -rot_z, pitch = -rot_y, roll = -rot_x.

M_TV_Screen parameters: Power (0/1), Picture (Texture2D / Media Texture), PictureGain, StaticAmount, StaticSpeed,
  ScanFreq, ScanStrength, Vignette, Tint, Glow.
"""
import os
import runpy
import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
FBX_DIR = os.path.join(HERE, "FBX")
TEX_DIR = os.path.normpath(os.path.join(HERE, "..", "..", "Textures"))
ROOT = os.environ.get("INTERIOR_ROOT", "/Game/Interior")  # e.g. /Game/Developers/mustafa/Interior for a personal sandbox
PBR_ROOT = os.environ.get("INTERIOR_PBR_ROOT", "/Game/PBRgen")
MESH_DIR = ROOT + "/Meshes"
MAT_DIR = ROOT + "/Materials"

ASSETS = unreal.EditorAssetLibrary
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
MEL = unreal.MaterialEditingLibrary
MP = unreal.MaterialProperty

# PBRgen materials -> Tiling (tiles per metre; props use metre-scale UVs)
PBR = {"M_Apt_Wood_Old": 1.6, "M_Apt_Metal_Rust": 1.6, "M_Apt_Fabric_Worn": 2.0, "M_Apt_Coat_Wool2": 3.0}
# plain slots: (base colour linear rgb, roughness, metallic)
SIMPLE = {
    "M_Apt_Brass": ((0.50, 0.36, 0.13), 0.40, 1.0),
    "M_Apt_Leather": ((0.05, 0.03, 0.025), 0.60, 0.0),
    "M_Apt_Mattress": ((0.07, 0.055, 0.04), 0.95, 0.0),
    "M_Apt_Metal": ((0.22, 0.22, 0.23), 0.55, 0.9),
    "M_Apt_Paper": ((0.32, 0.30, 0.25), 0.90, 0.0),
    "M_Apt_Teak": ((0.30, 0.17, 0.07), 0.55, 0.0),
    "M_Award_Gold": ((0.62, 0.42, 0.12), 0.40, 1.0),
    "M_Award_Label": ((0.20, 0.15, 0.07), 0.55, 0.8),
    "M_Bed_Sheet": ((0.50, 0.46, 0.38), 0.92, 0.0),
    "M_Counter_Top": ((0.34, 0.32, 0.28), 0.40, 0.0),
    "M_Fridge_Enamel": ((0.50, 0.46, 0.37), 0.38, 0.1),
    "M_Fridge_Liner": ((0.30, 0.27, 0.19), 0.60, 0.0),
    "M_Note_Paper": ((0.62, 0.58, 0.45), 0.95, 0.0),
    "M_TV_Knob": ((0.18, 0.17, 0.15), 0.45, 0.0),
    "M_TV_Plastic": ((0.07, 0.055, 0.045), 0.55, 0.0),
    "M_Wardrobe_Mirror": ((0.30, 0.33, 0.34), 0.06, 1.0),
}


def node(material, kind, x, y, **props):
    e = MEL.create_material_expression(material, kind, x, y)
    for k, v in props.items():
        e.set_editor_property(k, v)
    return e


def link(src, out, dst, inp):
    if not MEL.connect_material_expressions(src, out, dst, inp):
        unreal.log_warning("Interior: could not connect to '%s'" % inp)


def scalar(m, name, value, x, y):
    return node(m, unreal.MaterialExpressionScalarParameter, x, y, parameter_name=name, default_value=value)


def mul(m, a, b, x, y, a_out="", b_out=""):
    n = node(m, unreal.MaterialExpressionMultiply, x, y)
    link(a, a_out, n, "A")
    link(b, b_out, n, "B")
    return n


def add(m, a, b, x, y):
    n = node(m, unreal.MaterialExpressionAdd, x, y)
    link(a, "", n, "A")
    link(b, "", n, "B")
    return n


# ---------------------------------------------------------------- materials
def build_pbrgen():
    out = {}
    for name, tiling in PBR.items():
        script = os.path.join(TEX_DIR, name, "import_pbrgen.py")
        if not os.path.exists(script):
            unreal.log_warning("Interior: missing PBRgen script " + script)
            continue
        with open(script, encoding="utf-8-sig") as fh:
            text = fh.read().lstrip("\ufeff").replace('ROOT = "/Game/PBRgen"', 'ROOT = "%s"' % PBR_ROOT)
        exec(compile(text, script, "exec"), {"__file__": script, "__name__": "__main__"})
        mi = ASSETS.load_asset("%s/%s/MI_%s" % (PBR_ROOT, name, name))
        if mi is None:
            unreal.log_warning("Interior: PBRgen instance not found for " + name)
            continue
        MEL.set_material_instance_scalar_parameter_value(mi, "Tiling", tiling)
        MEL.update_material_instance(mi)
        ASSETS.save_loaded_asset(mi)
        out[name] = mi
    return out


def build_basic_master():
    path = MAT_DIR + "/M_Prop_Basic"
    if ASSETS.does_asset_exist(path):
        return ASSETS.load_asset(path)
    m = TOOLS.create_asset("M_Prop_Basic", MAT_DIR, unreal.Material, unreal.MaterialFactoryNew())
    col = node(m, unreal.MaterialExpressionVectorParameter, -500, -200, parameter_name="BaseColor",
               default_value=unreal.LinearColor(0.1, 0.1, 0.1, 1.0))
    rough = scalar(m, "Roughness", 0.5, -500, 0)
    metal = scalar(m, "Metallic", 0.0, -500, 120)
    MEL.connect_material_property(col, "", MP.MP_BASE_COLOR)
    MEL.connect_material_property(rough, "", MP.MP_ROUGHNESS)
    MEL.connect_material_property(metal, "", MP.MP_METALLIC)
    MEL.recompile_material(m)
    ASSETS.save_loaded_asset(m)
    return m


def build_simple_instances(master):
    out = {}
    for name, (rgb, rough, metal) in SIMPLE.items():
        path = MAT_DIR + "/MI_" + name
        if ASSETS.does_asset_exist(path):
            mi = ASSETS.load_asset(path)
        else:
            mi = TOOLS.create_asset("MI_" + name, MAT_DIR, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        MEL.set_material_instance_parent(mi, master)
        MEL.set_material_instance_vector_parameter_value(mi, "BaseColor", unreal.LinearColor(rgb[0], rgb[1], rgb[2], 1.0))
        MEL.set_material_instance_scalar_parameter_value(mi, "Roughness", rough)
        MEL.set_material_instance_scalar_parameter_value(mi, "Metallic", metal)
        MEL.update_material_instance(mi)
        ASSETS.save_loaded_asset(mi)
        out[name] = mi
    return out


def build_screen_master():
    path = MAT_DIR + "/M_TV_Screen"
    if ASSETS.does_asset_exist(path):
        ASSETS.delete_asset(path)
    m = TOOLS.create_asset("M_TV_Screen", MAT_DIR, unreal.Material, unreal.MaterialFactoryNew())
    uv = node(m, unreal.MaterialExpressionTextureCoordinate, -2000, 0, coordinate_index=0)
    time = node(m, unreal.MaterialExpressionTime, -2000, 400)

    picture = node(m, unreal.MaterialExpressionTextureSampleParameter2D, -1500, -400, parameter_name="Picture",
                   sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
    try:
        black = ASSETS.load_asset("/Engine/EngineResources/Black")
        if black:
            picture.set_editor_property("texture", black)
    except Exception as error:
        unreal.log_warning("Interior: default picture: %s" % error)
    link(uv, "", picture, "UVs")
    gain = scalar(m, "PictureGain", 1.5, -1500, -150)
    pic = mul(m, picture, gain, -1200, -350, "RGB", "")

    speed = scalar(m, "StaticSpeed", 30.0, -1700, 520)
    tmul = mul(m, time, speed, -1500, 480)
    pos = node(m, unreal.MaterialExpressionAppendVector, -1300, 300)
    link(uv, "", pos, "A")
    link(tmul, "", pos, "B")
    noise = node(m, unreal.MaterialExpressionNoise, -1100, 300)
    for k, v in (("scale", 180.0), ("quality", 1), ("levels", 1), ("output_min", 0.0), ("output_max", 1.0)):
        try:
            noise.set_editor_property(k, v)
        except Exception:
            pass
    if not MEL.connect_material_expressions(pos, "", noise, "Position"):
        MEL.connect_material_expressions(pos, "", noise, "")
    amount = scalar(m, "StaticAmount", 0.35, -1100, 520)
    snow = mul(m, noise, amount, -850, 360)

    vmask = node(m, unreal.MaterialExpressionComponentMask, -1500, 700, r=False, g=True, b=False, a=False)
    link(uv, "", vmask, "")
    freq = scalar(m, "ScanFreq", 1500.0, -1500, 880)
    vf = mul(m, vmask, freq, -1250, 760)
    sine = node(m, unreal.MaterialExpressionSine, -1050, 760)
    link(vf, "", sine, "")
    strength = scalar(m, "ScanStrength", 0.12, -1050, 900)
    s1 = mul(m, sine, strength, -820, 780)
    one = node(m, unreal.MaterialExpressionConstant, -820, 900, r=1.0)
    scan = add(m, s1, one, -600, 800)

    half = node(m, unreal.MaterialExpressionConstant2Vector, -1500, 1100, r=0.5, g=0.5)
    dist = node(m, unreal.MaterialExpressionDistance, -1250, 1100)
    link(uv, "", dist, "A")
    link(half, "", dist, "B")
    vgn = scalar(m, "Vignette", 1.4, -1250, 1250)
    dv = mul(m, dist, vgn, -1000, 1120)
    inv = node(m, unreal.MaterialExpressionOneMinus, -800, 1120)
    link(dv, "", inv, "")
    sat = node(m, unreal.MaterialExpressionSaturate, -600, 1120)
    link(inv, "", sat, "")

    f37 = node(m, unreal.MaterialExpressionConstant, -1500, 1450, r=37.0)
    ft = mul(m, time, f37, -1300, 1450)
    fs = node(m, unreal.MaterialExpressionSine, -1100, 1450)
    link(ft, "", fs, "")
    fa = node(m, unreal.MaterialExpressionConstant, -1100, 1580, r=0.025)
    fm = mul(m, fs, fa, -900, 1470)
    f1 = node(m, unreal.MaterialExpressionConstant, -900, 1600, r=0.975)
    flicker = add(m, fm, f1, -700, 1500)

    glow = scalar(m, "Glow", 0.04, -600, 400)
    tint = node(m, unreal.MaterialExpressionVectorParameter, -600, 560, parameter_name="Tint",
                default_value=unreal.LinearColor(0.78, 0.9, 1.0, 1.0))
    base = add(m, pic, snow, -600, -100)
    base = add(m, base, glow, -400, -60)
    tinted = mul(m, base, tint, -200, 0, "", "RGB")
    x1 = mul(m, tinted, scan, 0, 0)
    x2 = mul(m, x1, sat, 200, 0)
    x3 = mul(m, x2, flicker, 400, 0)
    power = scalar(m, "Power", 1.0, 400, 160)
    final = mul(m, x3, power, 600, 0)
    MEL.connect_material_property(final, "", MP.MP_EMISSIVE_COLOR)
    black_c = node(m, unreal.MaterialExpressionConstant3Vector, 600, -300, constant=unreal.LinearColor(0.006, 0.007, 0.008, 1.0))
    MEL.connect_material_property(black_c, "", MP.MP_BASE_COLOR)
    rough = node(m, unreal.MaterialExpressionConstant, 600, -200, r=0.08)
    MEL.connect_material_property(rough, "", MP.MP_ROUGHNESS)
    MEL.recompile_material(m)
    ASSETS.save_loaded_asset(m)

    mi = TOOLS.create_asset("MI_TV_Screen", MAT_DIR, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew()) \
        if not ASSETS.does_asset_exist(MAT_DIR + "/MI_TV_Screen") else ASSETS.load_asset(MAT_DIR + "/MI_TV_Screen")
    MEL.set_material_instance_parent(mi, m)
    MEL.update_material_instance(mi)
    ASSETS.save_loaded_asset(mi)
    return mi


# ---------------------------------------------------------------- meshes
def import_meshes():
    tasks = []
    names = []
    for f in sorted(os.listdir(FBX_DIR)):
        if not f.lower().endswith(".fbx"):
            continue
        t = unreal.AssetImportTask()
        t.set_editor_property("filename", os.path.join(FBX_DIR, f))
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
        opt.static_mesh_import_data.set_editor_property("combine_meshes", True)
        t.set_editor_property("options", opt)
        tasks.append(t)
        names.append(os.path.splitext(f)[0])
    TOOLS.import_asset_tasks(tasks)
    return names


def assign_materials(names, mapping):
    missing = set()
    for n in names:
        sm = ASSETS.load_asset(MESH_DIR + "/" + n)
        if sm is None:
            unreal.log_warning("Interior: mesh not found " + n)
            continue
        mats = list(sm.get_editor_property("static_materials"))
        for slot in mats:
            sname = str(slot.get_editor_property("material_slot_name"))
            if sname in mapping:
                slot.set_editor_property("material_interface", mapping[sname])
            else:
                missing.add(sname)
        sm.set_editor_property("static_materials", mats)
        ASSETS.save_loaded_asset(sm)
    if missing:
        unreal.log_warning("Interior: unmapped material slots: %s" % sorted(missing))


def main():
    pbr = build_pbrgen()
    basic = build_basic_master()
    mapping = dict(pbr)
    mapping.update(build_simple_instances(basic))
    mapping["M_TV_Screen"] = build_screen_master()
    names = import_meshes()
    assign_materials(names, mapping)
    unreal.log("Interior: imported %d meshes, %d materials" % (len(names), len(mapping)))


main()

