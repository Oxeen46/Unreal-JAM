PBRgen for Unreal Engine and UEFN

Unzip the whole download first.

UNREAL ENGINE 5
  1. Once per project: Edit > Plugins, enable "Python Editor Script Plugin",
     and restart the editor.
  2. Tools > Execute Python Script..., and pick import_pbrgen.py in this folder.
  3. The material is in Content/PBRgen/M_Concrete: drag MI_M_Concrete onto a mesh.
  The material instance has DisplacementAmount (in centimeters), DisplacementZero,
  NormalStrength, RoughnessMultiplier and Tiling.

DISPLACEMENT
  Needs Unreal 5.3 or later and a mesh with Nanite enabled or enough
  triangles. The starting amount assumes one tile is 1 meter.

UEFN, OR BUILDING THE MATERIAL BY HAND
  Import the T_ textures and set them up as follows:
    T_M_Concrete_BC    Default, sRGB on      Base Color
    T_M_Concrete_N     Normalmap, DirectX    Normal
    T_M_Concrete_ORM   Masks, sRGB off       R Ambient Occlusion, G Roughness, B Metallic
    T_M_Concrete_H     Grayscale, sRGB off   Displacement

Coat, sheen and other OpenPBR maps are not part of this export. Pick no
software in PBRgen to get every map, with a .mtlx and a .usda.
