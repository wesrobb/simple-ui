local obj_0 = o("Glyph Outline Style");
local obj_1 = o("Glyph Body Style");
local obj_2 = o("Glyph Shadow Style");
local obj_3 = o("Glyph Outline");
local obj_4 = o("Glyph Shadow");
local obj_5 = o("Glyph Typography");
local obj_6 = o("Glyph");

/* Object Properties */

obj_0 + s(pdRGBT, "50641400");

obj_1 + s(pdRGBT, "B4961400");
obj_1 + s(pdEP, "030A");

obj_2 + s(pdRGBT, "80808078");
obj_2 + s(pdEP, "030A");

obj_3 + l(pdStylePtr, obj_0);
obj_3 + i(pdLineThickness, 240);

obj_4 + l(pdStylePtr, obj_2);
obj_4 + i(pdShadowDx, 5);
obj_4 + i(pdShadowDy, 5);

obj_5 + l(pdBodyPtr, obj_1);
obj_5 + l(pdShadowPtr, obj_4);
obj_5 + l(pdOutlinePtr, obj_3);
obj_5 + i(pdFontIndex, 0);
obj_5 + i(pdFontDw, 190);
obj_5 + i(pdFontDh, 190);

obj_6 + l(pdTypographyPtr, obj_5);
obj_6 + i(pdID, 85);

/* Image Pinning */

obj_6.Pin(0, 50, 250, pdDrawGlyph);
