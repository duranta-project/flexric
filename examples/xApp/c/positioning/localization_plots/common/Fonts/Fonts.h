/*

Copyright (c) 2022 Evan Pezent & ImPlot Community

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#pragma once

#include "Fonts/IconsFontAwesome5.h"
#include "Fonts/IconsFontAwesome5Brands.h"

// Embedded fonts. The data for each font can be loaded by ImGui.
//
// ImFontConfig font_cfg;
// font_cfg.FontDataOwnedByAtlas = false;
// ... more font_cfg params.
// ImGui::GetIO().Fonts->AddFontFromMemoryTTF(Roboto_Regular_ttf, Roboto_Regular_ttf_len, 14.0f,
// &font_cfg);
//
// The font data was generated with:
// Bash: xxd -i Roboto-Regular.ttf > font_data.txt

//==============================================================================
// ROBOTO
//==============================================================================

extern unsigned char Roboto_Regular_ttf[];
extern unsigned int  Roboto_Regular_ttf_len;

extern unsigned char Roboto_Bold_ttf[];
extern unsigned int  Roboto_Bold_ttf_len;

extern unsigned char Roboto_Italic_ttf[];
extern unsigned int  Roboto_Italic_ttf_len;

//==============================================================================
// ROBOTO MONO
//==============================================================================

extern unsigned char RobotoMono_Regular_ttf[];
extern unsigned int  RobotoMono_Regular_ttf_len;

extern unsigned char RobotoMono_Bold_ttf[];
extern unsigned int  RobotoMono_Bold_ttf_len;

extern unsigned char RobotoMono_Italic_ttf[];
extern unsigned int  RobotoMono_Italic_ttf_len;

//==============================================================================
// FONT AWESOME
//==============================================================================

extern unsigned char fa_solid_900_ttf[];
extern unsigned int  fa_solid_900_ttf_len;

extern unsigned char fa_brands_400_ttf[];
extern unsigned int  fa_brands_400_ttf_len;
