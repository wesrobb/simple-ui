echo -------------------------------------------------------------------------------
echo Build example_color_rasterizer.exe
echo -------------------------------------------------------------------------------

echo --- Step 1: Compile ---
cl.exe /W4 /nologo /c /TP /std:c++17 /openmp- /permissive- /Zc:forScope /Zc:inline /GS- /GR- /O2 /Ob2 /Oi /Ot /D "_CRT_SECURE_NO_DEPRECATE" /D "WIN32" /D "_WINDOWS" /D "_CONSOLE" /D "NDEBUG" /D "_MBCS" /D "_LIB" /I "../../../includes/core-cpp/" /I "../../../includes/utils/" /Fp"example_color_rasterizer.pch" /c "..\..\..\examples-cpp\example_color_rasterizer\main.cpp"
echo --- Step 2: Link ---
link.exe kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"example_color_rasterizer.pdb" /machine:%MACHINE% /out:"example_color_rasterizer.exe" "main.obj" "..\..\..\engine\libs\%LIBPREFIX_DTYPE%\dtypestd%LIBEXTENSION_DTYPE%" "..\..\..\other\libs\%LIBPREFIX_OTHER%\dtwindow.lib"
echo Done.
echo -------------------------------------------------------------------------------
