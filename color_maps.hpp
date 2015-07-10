#ifndef SASS_COLOR_MAPS_H
#define SASS_COLOR_MAPS_H

#include <map>
#include "ast.hpp"

namespace Sass {

  namespace Colors {
    static Color aliceblue(ParserState("[COLOR TABLE]"), 240, 248, 255, 1);
    static Color antiquewhite(ParserState("[COLOR TABLE]"), 250, 235, 215, 1);
    static Color cyan(ParserState("[COLOR TABLE]"), 0, 255, 255, 1);
    static Color aqua(ParserState("[COLOR TABLE]"), 0, 255, 255, 1);
    static Color aquamarine(ParserState("[COLOR TABLE]"), 127, 255, 212, 1);
    static Color azure(ParserState("[COLOR TABLE]"), 240, 255, 255, 1);
    static Color beige(ParserState("[COLOR TABLE]"), 245, 245, 220, 1);
    static Color bisque(ParserState("[COLOR TABLE]"), 255, 228, 196, 1);
    static Color black(ParserState("[COLOR TABLE]"), 0, 0, 0, 1);
    static Color blanchedalmond(ParserState("[COLOR TABLE]"), 255, 235, 205, 1);
    static Color blue(ParserState("[COLOR TABLE]"), 0, 0, 255, 1);
    static Color blueviolet(ParserState("[COLOR TABLE]"), 138, 43, 226, 1);
    static Color brown(ParserState("[COLOR TABLE]"), 165, 42, 42, 1);
    static Color burlywood(ParserState("[COLOR TABLE]"), 222, 184, 135, 1);
    static Color cadetblue(ParserState("[COLOR TABLE]"), 95, 158, 160, 1);
    static Color chartreuse(ParserState("[COLOR TABLE]"), 127, 255, 0, 1);
    static Color chocolate(ParserState("[COLOR TABLE]"), 210, 105, 30, 1);
    static Color coral(ParserState("[COLOR TABLE]"), 255, 127, 80, 1);
    static Color cornflowerblue(ParserState("[COLOR TABLE]"), 100, 149, 237, 1);
    static Color cornsilk(ParserState("[COLOR TABLE]"), 255, 248, 220, 1);
    static Color crimson(ParserState("[COLOR TABLE]"), 220, 20, 60, 1);
    static Color darkblue(ParserState("[COLOR TABLE]"), 0, 0, 139, 1);
    static Color darkcyan(ParserState("[COLOR TABLE]"), 0, 139, 139, 1);
    static Color darkgoldenrod(ParserState("[COLOR TABLE]"), 184, 134, 11, 1);
    static Color darkgray(ParserState("[COLOR TABLE]"), 169, 169, 169, 1);
    static Color darkgrey(ParserState("[COLOR TABLE]"), 169, 169, 169, 1);
    static Color darkgreen(ParserState("[COLOR TABLE]"), 0, 100, 0, 1);
    static Color darkkhaki(ParserState("[COLOR TABLE]"), 189, 183, 107, 1);
    static Color darkmagenta(ParserState("[COLOR TABLE]"), 139, 0, 139, 1);
    static Color darkolivegreen(ParserState("[COLOR TABLE]"), 85, 107, 47, 1);
    static Color darkorange(ParserState("[COLOR TABLE]"), 255, 140, 0, 1);
    static Color darkorchid(ParserState("[COLOR TABLE]"), 153, 50, 204, 1);
    static Color darkred(ParserState("[COLOR TABLE]"), 139, 0, 0, 1);
    static Color darksalmon(ParserState("[COLOR TABLE]"), 233, 150, 122, 1);
    static Color darkseagreen(ParserState("[COLOR TABLE]"), 143, 188, 143, 1);
    static Color darkslateblue(ParserState("[COLOR TABLE]"), 72, 61, 139, 1);
    static Color darkslategray(ParserState("[COLOR TABLE]"), 47, 79, 79, 1);
    static Color darkslategrey(ParserState("[COLOR TABLE]"), 47, 79, 79, 1);
    static Color darkturquoise(ParserState("[COLOR TABLE]"), 0, 206, 209, 1);
    static Color darkviolet(ParserState("[COLOR TABLE]"), 148, 0, 211, 1);
    static Color deeppink(ParserState("[COLOR TABLE]"), 255, 20, 147, 1);
    static Color deepskyblue(ParserState("[COLOR TABLE]"), 0, 191, 255, 1);
    static Color dimgray(ParserState("[COLOR TABLE]"), 105, 105, 105, 1);
    static Color dimgrey(ParserState("[COLOR TABLE]"), 105, 105, 105, 1);
    static Color dodgerblue(ParserState("[COLOR TABLE]"), 30, 144, 255, 1);
    static Color firebrick(ParserState("[COLOR TABLE]"), 178, 34, 34, 1);
    static Color floralwhite(ParserState("[COLOR TABLE]"), 255, 250, 240, 1);
    static Color forestgreen(ParserState("[COLOR TABLE]"), 34, 139, 34, 1);
    static Color magenta(ParserState("[COLOR TABLE]"), 255, 0, 255, 1);
    static Color fuchsia(ParserState("[COLOR TABLE]"), 255, 0, 255, 1);
    static Color gainsboro(ParserState("[COLOR TABLE]"), 220, 220, 220, 1);
    static Color ghostwhite(ParserState("[COLOR TABLE]"), 248, 248, 255, 1);
    static Color gold(ParserState("[COLOR TABLE]"), 255, 215, 0, 1);
    static Color goldenrod(ParserState("[COLOR TABLE]"), 218, 165, 32, 1);
    static Color gray(ParserState("[COLOR TABLE]"), 128, 128, 128, 1);
    static Color grey(ParserState("[COLOR TABLE]"), 128, 128, 128, 1);
    static Color green(ParserState("[COLOR TABLE]"), 0, 128, 0, 1);
    static Color greenyellow(ParserState("[COLOR TABLE]"), 173, 255, 47, 1);
    static Color honeydew(ParserState("[COLOR TABLE]"), 240, 255, 240, 1);
    static Color hotpink(ParserState("[COLOR TABLE]"), 255, 105, 180, 1);
    static Color indianred(ParserState("[COLOR TABLE]"), 205, 92, 92, 1);
    static Color indigo(ParserState("[COLOR TABLE]"), 75, 0, 130, 1);
    static Color ivory(ParserState("[COLOR TABLE]"), 255, 255, 240, 1);
    static Color khaki(ParserState("[COLOR TABLE]"), 240, 230, 140, 1);
    static Color lavender(ParserState("[COLOR TABLE]"), 230, 230, 250, 1);
    static Color lavenderblush(ParserState("[COLOR TABLE]"), 255, 240, 245, 1);
    static Color lawngreen(ParserState("[COLOR TABLE]"), 124, 252, 0, 1);
    static Color lemonchiffon(ParserState("[COLOR TABLE]"), 255, 250, 205, 1);
    static Color lightblue(ParserState("[COLOR TABLE]"), 173, 216, 230, 1);
    static Color lightcoral(ParserState("[COLOR TABLE]"), 240, 128, 128, 1);
    static Color lightcyan(ParserState("[COLOR TABLE]"), 224, 255, 255, 1);
    static Color lightgoldenrodyellow(ParserState("[COLOR TABLE]"), 250, 250, 210, 1);
    static Color lightgray(ParserState("[COLOR TABLE]"), 211, 211, 211, 1);
    static Color lightgrey(ParserState("[COLOR TABLE]"), 211, 211, 211, 1);
    static Color lightgreen(ParserState("[COLOR TABLE]"), 144, 238, 144, 1);
    static Color lightpink(ParserState("[COLOR TABLE]"), 255, 182, 193, 1);
    static Color lightsalmon(ParserState("[COLOR TABLE]"), 255, 160, 122, 1);
    static Color lightseagreen(ParserState("[COLOR TABLE]"), 32, 178, 170, 1);
    static Color lightskyblue(ParserState("[COLOR TABLE]"), 135, 206, 250, 1);
    static Color lightslategray(ParserState("[COLOR TABLE]"), 119, 136, 153, 1);
    static Color lightslategrey(ParserState("[COLOR TABLE]"), 119, 136, 153, 1);
    static Color lightsteelblue(ParserState("[COLOR TABLE]"), 176, 196, 222, 1);
    static Color lightyellow(ParserState("[COLOR TABLE]"), 255, 255, 224, 1);
    static Color lime(ParserState("[COLOR TABLE]"), 0, 255, 0, 1);
    static Color limegreen(ParserState("[COLOR TABLE]"), 50, 205, 50, 1);
    static Color linen(ParserState("[COLOR TABLE]"), 250, 240, 230, 1);
    static Color maroon(ParserState("[COLOR TABLE]"), 128, 0, 0, 1);
    static Color mediumaquamarine(ParserState("[COLOR TABLE]"), 102, 205, 170, 1);
    static Color mediumblue(ParserState("[COLOR TABLE]"), 0, 0, 205, 1);
    static Color mediumorchid(ParserState("[COLOR TABLE]"), 186, 85, 211, 1);
    static Color mediumpurple(ParserState("[COLOR TABLE]"), 147, 112, 219, 1);
    static Color mediumseagreen(ParserState("[COLOR TABLE]"), 60, 179, 113, 1);
    static Color mediumslateblue(ParserState("[COLOR TABLE]"), 123, 104, 238, 1);
    static Color mediumspringgreen(ParserState("[COLOR TABLE]"), 0, 250, 154, 1);
    static Color mediumturquoise(ParserState("[COLOR TABLE]"), 72, 209, 204, 1);
    static Color mediumvioletred(ParserState("[COLOR TABLE]"), 199, 21, 133, 1);
    static Color midnightblue(ParserState("[COLOR TABLE]"), 25, 25, 112, 1);
    static Color mintcream(ParserState("[COLOR TABLE]"), 245, 255, 250, 1);
    static Color mistyrose(ParserState("[COLOR TABLE]"), 255, 228, 225, 1);
    static Color moccasin(ParserState("[COLOR TABLE]"), 255, 228, 181, 1);
    static Color navajowhite(ParserState("[COLOR TABLE]"), 255, 222, 173, 1);
    static Color navy(ParserState("[COLOR TABLE]"), 0, 0, 128, 1);
    static Color oldlace(ParserState("[COLOR TABLE]"), 253, 245, 230, 1);
    static Color olive(ParserState("[COLOR TABLE]"), 128, 128, 0, 1);
    static Color olivedrab(ParserState("[COLOR TABLE]"), 107, 142, 35, 1);
    static Color orange(ParserState("[COLOR TABLE]"), 255, 165, 0, 1);
    static Color orangered(ParserState("[COLOR TABLE]"), 255, 69, 0, 1);
    static Color orchid(ParserState("[COLOR TABLE]"), 218, 112, 214, 1);
    static Color palegoldenrod(ParserState("[COLOR TABLE]"), 238, 232, 170, 1);
    static Color palegreen(ParserState("[COLOR TABLE]"), 152, 251, 152, 1);
    static Color paleturquoise(ParserState("[COLOR TABLE]"), 175, 238, 238, 1);
    static Color palevioletred(ParserState("[COLOR TABLE]"), 219, 112, 147, 1);
    static Color papayawhip(ParserState("[COLOR TABLE]"), 255, 239, 213, 1);
    static Color peachpuff(ParserState("[COLOR TABLE]"), 255, 218, 185, 1);
    static Color peru(ParserState("[COLOR TABLE]"), 205, 133, 63, 1);
    static Color pink(ParserState("[COLOR TABLE]"), 255, 192, 203, 1);
    static Color plum(ParserState("[COLOR TABLE]"), 221, 160, 221, 1);
    static Color powderblue(ParserState("[COLOR TABLE]"), 176, 224, 230, 1);
    static Color purple(ParserState("[COLOR TABLE]"), 128, 0, 128, 1);
    static Color red(ParserState("[COLOR TABLE]"), 255, 0, 0, 1);
    static Color rosybrown(ParserState("[COLOR TABLE]"), 188, 143, 143, 1);
    static Color royalblue(ParserState("[COLOR TABLE]"), 65, 105, 225, 1);
    static Color saddlebrown(ParserState("[COLOR TABLE]"), 139, 69, 19, 1);
    static Color salmon(ParserState("[COLOR TABLE]"), 250, 128, 114, 1);
    static Color sandybrown(ParserState("[COLOR TABLE]"), 244, 164, 96, 1);
    static Color seagreen(ParserState("[COLOR TABLE]"), 46, 139, 87, 1);
    static Color seashell(ParserState("[COLOR TABLE]"), 255, 245, 238, 1);
    static Color sienna(ParserState("[COLOR TABLE]"), 160, 82, 45, 1);
    static Color silver(ParserState("[COLOR TABLE]"), 192, 192, 192, 1);
    static Color skyblue(ParserState("[COLOR TABLE]"), 135, 206, 235, 1);
    static Color slateblue(ParserState("[COLOR TABLE]"), 106, 90, 205, 1);
    static Color slategray(ParserState("[COLOR TABLE]"), 112, 128, 144, 1);
    static Color slategrey(ParserState("[COLOR TABLE]"), 112, 128, 144, 1);
    static Color snow(ParserState("[COLOR TABLE]"), 255, 250, 250, 1);
    static Color springgreen(ParserState("[COLOR TABLE]"), 0, 255, 127, 1);
    static Color steelblue(ParserState("[COLOR TABLE]"), 70, 130, 180, 1);
    static Color tan(ParserState("[COLOR TABLE]"), 210, 180, 140, 1);
    static Color teal(ParserState("[COLOR TABLE]"), 0, 128, 128, 1);
    static Color thistle(ParserState("[COLOR TABLE]"), 216, 191, 216, 1);
    static Color tomato(ParserState("[COLOR TABLE]"), 255, 99, 71, 1);
    static Color turquoise(ParserState("[COLOR TABLE]"), 64, 224, 208, 1);
    static Color violet(ParserState("[COLOR TABLE]"), 238, 130, 238, 1);
    static Color wheat(ParserState("[COLOR TABLE]"), 245, 222, 179, 1);
    static Color white(ParserState("[COLOR TABLE]"), 255, 255, 255, 1);
    static Color whitesmoke(ParserState("[COLOR TABLE]"), 245, 245, 245, 1);
    static Color yellow(ParserState("[COLOR TABLE]"), 255, 255, 0, 1);
    static Color yellowgreen(ParserState("[COLOR TABLE]"), 154, 205, 50, 1);
    static Color rebeccapurple(ParserState("[COLOR TABLE]"), 102, 51, 153, 1);
    static Color transparent(ParserState("[COLOR TABLE]"), 0, 0, 0, 0);
  }

  const std::map<const int, const std::string> colors_to_names {
    { static_cast<int>(240) * 0x10000 + static_cast<int>(248) * 0x100 + static_cast<int>(255), "aliceblue" },
    { static_cast<int>(250) * 0x10000 + static_cast<int>(235) * 0x100 + static_cast<int>(215), "antiquewhite" },
    { static_cast<int>(  0) * 0x10000 + static_cast<int>(255) * 0x100 + static_cast<int>(255), "cyan" },
    { static_cast<int>(127) * 0x10000 + static_cast<int>(255) * 0x100 + static_cast<int>(212), "aquamarine" },
    { static_cast<int>(240) * 0x10000 + static_cast<int>(255) * 0x100 + static_cast<int>(255), "azure" },
    { static_cast<int>(245) * 0x10000 + static_cast<int>(245) * 0x100 + static_cast<int>(220), "beige" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(228) * 0x100 + static_cast<int>(196), "bisque" },
    { static_cast<int>(  0) * 0x10000 + static_cast<int>(  0) * 0x100 + static_cast<int>(  0), "black" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(235) * 0x100 + static_cast<int>(205), "blanchedalmond" },
    { static_cast<int>(  0) * 0x10000 + static_cast<int>(  0) * 0x100 + static_cast<int>(255), "blue" },
    { static_cast<int>(138) * 0x10000 + static_cast<int>( 43) * 0x100 + static_cast<int>(226), "blueviolet" },
    { static_cast<int>(165) * 0x10000 + static_cast<int>( 42) * 0x100 + static_cast<int>( 42), "brown" },
    { static_cast<int>(222) * 0x10000 + static_cast<int>(184) * 0x100 + static_cast<int>(135), "burlywood" },
    { static_cast<int>( 95) * 0x10000 + static_cast<int>(158) * 0x100 + static_cast<int>(160), "cadetblue" },
    { static_cast<int>(127) * 0x10000 + static_cast<int>(255) * 0x100 + static_cast<int>(  0), "chartreuse" },
    { static_cast<int>(210) * 0x10000 + static_cast<int>(105) * 0x100 + static_cast<int>( 30), "chocolate" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(127) * 0x100 + static_cast<int>( 80), "coral" },
    { static_cast<int>(100) * 0x10000 + static_cast<int>(149) * 0x100 + static_cast<int>(237), "cornflowerblue" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(248) * 0x100 + static_cast<int>(220), "cornsilk" },
    { static_cast<int>(220) * 0x10000 + static_cast<int>( 20) * 0x100 + static_cast<int>( 60), "crimson" },
    { static_cast<int>(  0) * 0x10000 + static_cast<int>(  0) * 0x100 + static_cast<int>(139), "darkblue" },
    { static_cast<int>(  0) * 0x10000 + static_cast<int>(139) * 0x100 + static_cast<int>(139), "darkcyan" },
    { static_cast<int>(184) * 0x10000 + static_cast<int>(134) * 0x100 + static_cast<int>( 11), "darkgoldenrod" },
    { static_cast<int>(169) * 0x10000 + static_cast<int>(169) * 0x100 + static_cast<int>(169), "darkgray" },
    { static_cast<int>(  0) * 0x10000 + static_cast<int>(100) * 0x100 + static_cast<int>(  0), "darkgreen" },
    { static_cast<int>(189) * 0x10000 + static_cast<int>(183) * 0x100 + static_cast<int>(107), "darkkhaki" },
    { static_cast<int>(139) * 0x10000 + static_cast<int>(  0) * 0x100 + static_cast<int>(139), "darkmagenta" },
    { static_cast<int>( 85) * 0x10000 + static_cast<int>(107) * 0x100 + static_cast<int>( 47), "darkolivegreen" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(140) * 0x100 + static_cast<int>(  0), "darkorange" },
    { static_cast<int>(153) * 0x10000 + static_cast<int>( 50) * 0x100 + static_cast<int>(204), "darkorchid" },
    { static_cast<int>(139) * 0x10000 + static_cast<int>(  0) * 0x100 + static_cast<int>(  0), "darkred" },
    { static_cast<int>(233) * 0x10000 + static_cast<int>(150) * 0x100 + static_cast<int>(122), "darksalmon" },
    { static_cast<int>(143) * 0x10000 + static_cast<int>(188) * 0x100 + static_cast<int>(143), "darkseagreen" },
    { static_cast<int>( 72) * 0x10000 + static_cast<int>( 61) * 0x100 + static_cast<int>(139), "darkslateblue" },
    { static_cast<int>( 47) * 0x10000 + static_cast<int>( 79) * 0x100 + static_cast<int>( 79), "darkslategray" },
    { static_cast<int>(  0) * 0x10000 + static_cast<int>(206) * 0x100 + static_cast<int>(209), "darkturquoise" },
    { static_cast<int>(148) * 0x10000 + static_cast<int>(  0) * 0x100 + static_cast<int>(211), "darkviolet" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>( 20) * 0x100 + static_cast<int>(147), "deeppink" },
    { static_cast<int>(  0) * 0x10000 + static_cast<int>(191) * 0x100 + static_cast<int>(255), "deepskyblue" },
    { static_cast<int>(105) * 0x10000 + static_cast<int>(105) * 0x100 + static_cast<int>(105), "dimgray" },
    { static_cast<int>( 30) * 0x10000 + static_cast<int>(144) * 0x100 + static_cast<int>(255), "dodgerblue" },
    { static_cast<int>(178) * 0x10000 + static_cast<int>( 34) * 0x100 + static_cast<int>( 34), "firebrick" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(250) * 0x100 + static_cast<int>(240), "floralwhite" },
    { static_cast<int>( 34) * 0x10000 + static_cast<int>(139) * 0x100 + static_cast<int>( 34), "forestgreen" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(  0) * 0x100 + static_cast<int>(255), "magenta" },
    { static_cast<int>(220) * 0x10000 + static_cast<int>(220) * 0x100 + static_cast<int>(220), "gainsboro" },
    { static_cast<int>(248) * 0x10000 + static_cast<int>(248) * 0x100 + static_cast<int>(255), "ghostwhite" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(215) * 0x100 + static_cast<int>(  0), "gold" },
    { static_cast<int>(218) * 0x10000 + static_cast<int>(165) * 0x100 + static_cast<int>( 32), "goldenrod" },
    { static_cast<int>(128) * 0x10000 + static_cast<int>(128) * 0x100 + static_cast<int>(128), "gray" },
    { static_cast<int>(  0) * 0x10000 + static_cast<int>(128) * 0x100 + static_cast<int>(  0), "green" },
    { static_cast<int>(173) * 0x10000 + static_cast<int>(255) * 0x100 + static_cast<int>( 47), "greenyellow" },
    { static_cast<int>(240) * 0x10000 + static_cast<int>(255) * 0x100 + static_cast<int>(240), "honeydew" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(105) * 0x100 + static_cast<int>(180), "hotpink" },
    { static_cast<int>(205) * 0x10000 + static_cast<int>( 92) * 0x100 + static_cast<int>( 92), "indianred" },
    { static_cast<int>( 75) * 0x10000 + static_cast<int>(  0) * 0x100 + static_cast<int>(130), "indigo" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(255) * 0x100 + static_cast<int>(240), "ivory" },
    { static_cast<int>(240) * 0x10000 + static_cast<int>(230) * 0x100 + static_cast<int>(140), "khaki" },
    { static_cast<int>(230) * 0x10000 + static_cast<int>(230) * 0x100 + static_cast<int>(250), "lavender" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(240) * 0x100 + static_cast<int>(245), "lavenderblush" },
    { static_cast<int>(124) * 0x10000 + static_cast<int>(252) * 0x100 + static_cast<int>(  0), "lawngreen" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(250) * 0x100 + static_cast<int>(205), "lemonchiffon" },
    { static_cast<int>(173) * 0x10000 + static_cast<int>(216) * 0x100 + static_cast<int>(230), "lightblue" },
    { static_cast<int>(240) * 0x10000 + static_cast<int>(128) * 0x100 + static_cast<int>(128), "lightcoral" },
    { static_cast<int>(224) * 0x10000 + static_cast<int>(255) * 0x100 + static_cast<int>(255), "lightcyan" },
    { static_cast<int>(250) * 0x10000 + static_cast<int>(250) * 0x100 + static_cast<int>(210), "lightgoldenrodyellow" },
    { static_cast<int>(211) * 0x10000 + static_cast<int>(211) * 0x100 + static_cast<int>(211), "lightgray" },
    { static_cast<int>(144) * 0x10000 + static_cast<int>(238) * 0x100 + static_cast<int>(144), "lightgreen" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(182) * 0x100 + static_cast<int>(193), "lightpink" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(160) * 0x100 + static_cast<int>(122), "lightsalmon" },
    { static_cast<int>( 32) * 0x10000 + static_cast<int>(178) * 0x100 + static_cast<int>(170), "lightseagreen" },
    { static_cast<int>(135) * 0x10000 + static_cast<int>(206) * 0x100 + static_cast<int>(250), "lightskyblue" },
    { static_cast<int>(119) * 0x10000 + static_cast<int>(136) * 0x100 + static_cast<int>(153), "lightslategray" },
    { static_cast<int>(176) * 0x10000 + static_cast<int>(196) * 0x100 + static_cast<int>(222), "lightsteelblue" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(255) * 0x100 + static_cast<int>(224), "lightyellow" },
    { static_cast<int>(  0) * 0x10000 + static_cast<int>(255) * 0x100 + static_cast<int>(  0), "lime" },
    { static_cast<int>( 50) * 0x10000 + static_cast<int>(205) * 0x100 + static_cast<int>( 50), "limegreen" },
    { static_cast<int>(250) * 0x10000 + static_cast<int>(240) * 0x100 + static_cast<int>(230), "linen" },
    { static_cast<int>(128) * 0x10000 + static_cast<int>(  0) * 0x100 + static_cast<int>(  0), "maroon" },
    { static_cast<int>(102) * 0x10000 + static_cast<int>(205) * 0x100 + static_cast<int>(170), "mediumaquamarine" },
    { static_cast<int>(  0) * 0x10000 + static_cast<int>(  0) * 0x100 + static_cast<int>(205), "mediumblue" },
    { static_cast<int>(186) * 0x10000 + static_cast<int>( 85) * 0x100 + static_cast<int>(211), "mediumorchid" },
    { static_cast<int>(147) * 0x10000 + static_cast<int>(112) * 0x100 + static_cast<int>(219), "mediumpurple" },
    { static_cast<int>( 60) * 0x10000 + static_cast<int>(179) * 0x100 + static_cast<int>(113), "mediumseagreen" },
    { static_cast<int>(123) * 0x10000 + static_cast<int>(104) * 0x100 + static_cast<int>(238), "mediumslateblue" },
    { static_cast<int>(  0) * 0x10000 + static_cast<int>(250) * 0x100 + static_cast<int>(154), "mediumspringgreen" },
    { static_cast<int>( 72) * 0x10000 + static_cast<int>(209) * 0x100 + static_cast<int>(204), "mediumturquoise" },
    { static_cast<int>(199) * 0x10000 + static_cast<int>( 21) * 0x100 + static_cast<int>(133), "mediumvioletred" },
    { static_cast<int>( 25) * 0x10000 + static_cast<int>( 25) * 0x100 + static_cast<int>(112), "midnightblue" },
    { static_cast<int>(245) * 0x10000 + static_cast<int>(255) * 0x100 + static_cast<int>(250), "mintcream" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(228) * 0x100 + static_cast<int>(225), "mistyrose" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(228) * 0x100 + static_cast<int>(181), "moccasin" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(222) * 0x100 + static_cast<int>(173), "navajowhite" },
    { static_cast<int>(  0) * 0x10000 + static_cast<int>(  0) * 0x100 + static_cast<int>(128), "navy" },
    { static_cast<int>(253) * 0x10000 + static_cast<int>(245) * 0x100 + static_cast<int>(230), "oldlace" },
    { static_cast<int>(128) * 0x10000 + static_cast<int>(128) * 0x100 + static_cast<int>(  0), "olive" },
    { static_cast<int>(107) * 0x10000 + static_cast<int>(142) * 0x100 + static_cast<int>( 35), "olivedrab" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(165) * 0x100 + static_cast<int>(  0), "orange" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>( 69) * 0x100 + static_cast<int>(  0), "orangered" },
    { static_cast<int>(218) * 0x10000 + static_cast<int>(112) * 0x100 + static_cast<int>(214), "orchid" },
    { static_cast<int>(238) * 0x10000 + static_cast<int>(232) * 0x100 + static_cast<int>(170), "palegoldenrod" },
    { static_cast<int>(152) * 0x10000 + static_cast<int>(251) * 0x100 + static_cast<int>(152), "palegreen" },
    { static_cast<int>(175) * 0x10000 + static_cast<int>(238) * 0x100 + static_cast<int>(238), "paleturquoise" },
    { static_cast<int>(219) * 0x10000 + static_cast<int>(112) * 0x100 + static_cast<int>(147), "palevioletred" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(239) * 0x100 + static_cast<int>(213), "papayawhip" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(218) * 0x100 + static_cast<int>(185), "peachpuff" },
    { static_cast<int>(205) * 0x10000 + static_cast<int>(133) * 0x100 + static_cast<int>( 63), "peru" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(192) * 0x100 + static_cast<int>(203), "pink" },
    { static_cast<int>(221) * 0x10000 + static_cast<int>(160) * 0x100 + static_cast<int>(221), "plum" },
    { static_cast<int>(176) * 0x10000 + static_cast<int>(224) * 0x100 + static_cast<int>(230), "powderblue" },
    { static_cast<int>(128) * 0x10000 + static_cast<int>(  0) * 0x100 + static_cast<int>(128), "purple" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(  0) * 0x100 + static_cast<int>(  0), "red" },
    { static_cast<int>(188) * 0x10000 + static_cast<int>(143) * 0x100 + static_cast<int>(143), "rosybrown" },
    { static_cast<int>( 65) * 0x10000 + static_cast<int>(105) * 0x100 + static_cast<int>(225), "royalblue" },
    { static_cast<int>(139) * 0x10000 + static_cast<int>( 69) * 0x100 + static_cast<int>( 19), "saddlebrown" },
    { static_cast<int>(250) * 0x10000 + static_cast<int>(128) * 0x100 + static_cast<int>(114), "salmon" },
    { static_cast<int>(244) * 0x10000 + static_cast<int>(164) * 0x100 + static_cast<int>( 96), "sandybrown" },
    { static_cast<int>( 46) * 0x10000 + static_cast<int>(139) * 0x100 + static_cast<int>( 87), "seagreen" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(245) * 0x100 + static_cast<int>(238), "seashell" },
    { static_cast<int>(160) * 0x10000 + static_cast<int>( 82) * 0x100 + static_cast<int>( 45), "sienna" },
    { static_cast<int>(192) * 0x10000 + static_cast<int>(192) * 0x100 + static_cast<int>(192), "silver" },
    { static_cast<int>(135) * 0x10000 + static_cast<int>(206) * 0x100 + static_cast<int>(235), "skyblue" },
    { static_cast<int>(106) * 0x10000 + static_cast<int>( 90) * 0x100 + static_cast<int>(205), "slateblue" },
    { static_cast<int>(112) * 0x10000 + static_cast<int>(128) * 0x100 + static_cast<int>(144), "slategray" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(250) * 0x100 + static_cast<int>(250), "snow" },
    { static_cast<int>(  0) * 0x10000 + static_cast<int>(255) * 0x100 + static_cast<int>(127), "springgreen" },
    { static_cast<int>( 70) * 0x10000 + static_cast<int>(130) * 0x100 + static_cast<int>(180), "steelblue" },
    { static_cast<int>(210) * 0x10000 + static_cast<int>(180) * 0x100 + static_cast<int>(140), "tan" },
    { static_cast<int>(  0) * 0x10000 + static_cast<int>(128) * 0x100 + static_cast<int>(128), "teal" },
    { static_cast<int>(216) * 0x10000 + static_cast<int>(191) * 0x100 + static_cast<int>(216), "thistle" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>( 99) * 0x100 + static_cast<int>( 71), "tomato" },
    { static_cast<int>( 64) * 0x10000 + static_cast<int>(224) * 0x100 + static_cast<int>(208), "turquoise" },
    { static_cast<int>(238) * 0x10000 + static_cast<int>(130) * 0x100 + static_cast<int>(238), "violet" },
    { static_cast<int>(245) * 0x10000 + static_cast<int>(222) * 0x100 + static_cast<int>(179), "wheat" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(255) * 0x100 + static_cast<int>(255), "white" },
    { static_cast<int>(245) * 0x10000 + static_cast<int>(245) * 0x100 + static_cast<int>(245), "whitesmoke" },
    { static_cast<int>(255) * 0x10000 + static_cast<int>(255) * 0x100 + static_cast<int>(  0), "yellow" },
    { static_cast<int>(154) * 0x10000 + static_cast<int>(205) * 0x100 + static_cast<int>( 50), "yellowgreen" },
    { static_cast<int>(102) * 0x10000 + static_cast<int>( 51) * 0x100 + static_cast<int>(153), "rebeccapurple" }
  };

  const std::map<const std::string, Color*> names_to_colors {
    { "aliceblue", &Colors::aliceblue },
    { "antiquewhite", &Colors::antiquewhite },
    { "cyan", &Colors::cyan },
    { "aqua", &Colors::aqua },
    { "aquamarine", &Colors::aquamarine },
    { "azure", &Colors::azure },
    { "beige", &Colors::beige },
    { "bisque", &Colors::bisque },
    { "black", &Colors::black },
    { "blanchedalmond", &Colors::blanchedalmond },
    { "blue", &Colors::blue },
    { "blueviolet", &Colors::blueviolet },
    { "brown", &Colors::brown },
    { "burlywood", &Colors::burlywood },
    { "cadetblue", &Colors::cadetblue },
    { "chartreuse", &Colors::chartreuse },
    { "chocolate", &Colors::chocolate },
    { "coral", &Colors::coral },
    { "cornflowerblue", &Colors::cornflowerblue },
    { "cornsilk", &Colors::cornsilk },
    { "crimson", &Colors::crimson },
    { "darkblue", &Colors::darkblue },
    { "darkcyan", &Colors::darkcyan },
    { "darkgoldenrod", &Colors::darkgoldenrod },
    { "darkgray", &Colors::darkgray },
    { "darkgrey", &Colors::darkgrey },
    { "darkgreen", &Colors::darkgreen },
    { "darkkhaki", &Colors::darkkhaki },
    { "darkmagenta", &Colors::darkmagenta },
    { "darkolivegreen", &Colors::darkolivegreen },
    { "darkorange", &Colors::darkorange },
    { "darkorchid", &Colors::darkorchid },
    { "darkred", &Colors::darkred },
    { "darksalmon", &Colors::darksalmon },
    { "darkseagreen", &Colors::darkseagreen },
    { "darkslateblue", &Colors::darkslateblue },
    { "darkslategray", &Colors::darkslategray },
    { "darkslategrey", &Colors::darkslategrey },
    { "darkturquoise", &Colors::darkturquoise },
    { "darkviolet", &Colors::darkviolet },
    { "deeppink", &Colors::deeppink },
    { "deepskyblue", &Colors::deepskyblue },
    { "dimgray", &Colors::dimgray },
    { "dimgrey", &Colors::dimgrey },
    { "dodgerblue", &Colors::dodgerblue },
    { "firebrick", &Colors::firebrick },
    { "floralwhite", &Colors::floralwhite },
    { "forestgreen", &Colors::forestgreen },
    { "magenta", &Colors::magenta },
    { "fuchsia", &Colors::fuchsia },
    { "gainsboro", &Colors::gainsboro },
    { "ghostwhite", &Colors::ghostwhite },
    { "gold", &Colors::gold },
    { "goldenrod", &Colors::goldenrod },
    { "gray", &Colors::gray },
    { "grey", &Colors::grey },
    { "green", &Colors::green },
    { "greenyellow", &Colors::greenyellow },
    { "honeydew", &Colors::honeydew },
    { "hotpink", &Colors::hotpink },
    { "indianred", &Colors::indianred },
    { "indigo", &Colors::indigo },
    { "ivory", &Colors::ivory },
    { "khaki", &Colors::khaki },
    { "lavender", &Colors::lavender },
    { "lavenderblush", &Colors::lavenderblush },
    { "lawngreen", &Colors::lawngreen },
    { "lemonchiffon", &Colors::lemonchiffon },
    { "lightblue", &Colors::lightblue },
    { "lightcoral", &Colors::lightcoral },
    { "lightcyan", &Colors::lightcyan },
    { "lightgoldenrodyellow", &Colors::lightgoldenrodyellow },
    { "lightgray", &Colors::lightgray },
    { "lightgrey", &Colors::lightgrey },
    { "lightgreen", &Colors::lightgreen },
    { "lightpink", &Colors::lightpink },
    { "lightsalmon", &Colors::lightsalmon },
    { "lightseagreen", &Colors::lightseagreen },
    { "lightskyblue", &Colors::lightskyblue },
    { "lightslategray", &Colors::lightslategray },
    { "lightslategrey", &Colors::lightslategrey },
    { "lightsteelblue", &Colors::lightsteelblue },
    { "lightyellow", &Colors::lightyellow },
    { "lime", &Colors::lime },
    { "limegreen", &Colors::limegreen },
    { "linen", &Colors::linen },
    { "maroon", &Colors::maroon },
    { "mediumaquamarine", &Colors::mediumaquamarine },
    { "mediumblue", &Colors::mediumblue },
    { "mediumorchid", &Colors::mediumorchid },
    { "mediumpurple", &Colors::mediumpurple },
    { "mediumseagreen", &Colors::mediumseagreen },
    { "mediumslateblue", &Colors::mediumslateblue },
    { "mediumspringgreen", &Colors::mediumspringgreen },
    { "mediumturquoise", &Colors::mediumturquoise },
    { "mediumvioletred", &Colors::mediumvioletred },
    { "midnightblue", &Colors::midnightblue },
    { "mintcream", &Colors::mintcream },
    { "mistyrose", &Colors::mistyrose },
    { "moccasin", &Colors::moccasin },
    { "navajowhite", &Colors::navajowhite },
    { "navy", &Colors::navy },
    { "oldlace", &Colors::oldlace },
    { "olive", &Colors::olive },
    { "olivedrab", &Colors::olivedrab },
    { "orange", &Colors::orange },
    { "orangered", &Colors::orangered },
    { "orchid", &Colors::orchid },
    { "palegoldenrod", &Colors::palegoldenrod },
    { "palegreen", &Colors::palegreen },
    { "paleturquoise", &Colors::paleturquoise },
    { "palevioletred", &Colors::palevioletred },
    { "papayawhip", &Colors::papayawhip },
    { "peachpuff", &Colors::peachpuff },
    { "peru", &Colors::peru },
    { "pink", &Colors::pink },
    { "plum", &Colors::plum },
    { "powderblue", &Colors::powderblue },
    { "purple", &Colors::purple },
    { "red", &Colors::red },
    { "rosybrown", &Colors::rosybrown },
    { "royalblue", &Colors::royalblue },
    { "saddlebrown", &Colors::saddlebrown },
    { "salmon", &Colors::salmon },
    { "sandybrown", &Colors::sandybrown },
    { "seagreen", &Colors::seagreen },
    { "seashell", &Colors::seashell },
    { "sienna", &Colors::sienna },
    { "silver", &Colors::silver },
    { "skyblue", &Colors::skyblue },
    { "slateblue", &Colors::slateblue },
    { "slategray", &Colors::slategray },
    { "slategrey", &Colors::slategrey },
    { "snow", &Colors::snow },
    { "springgreen", &Colors::springgreen },
    { "steelblue", &Colors::steelblue },
    { "tan", &Colors::tan },
    { "teal", &Colors::teal },
    { "thistle", &Colors::thistle },
    { "tomato", &Colors::tomato },
    { "turquoise", &Colors::turquoise },
    { "violet", &Colors::violet },
    { "wheat", &Colors::wheat },
    { "white", &Colors::white },
    { "whitesmoke", &Colors::whitesmoke },
    { "yellow", &Colors::yellow },
    { "yellowgreen", &Colors::yellowgreen },
    { "rebeccapurple", &Colors::rebeccapurple },
    { "transparent", &Colors::transparent }
  };

}

#endif
