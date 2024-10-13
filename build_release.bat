@echo [%time%] Started
scons platform=windows
scons platform=windows target=template_release
REM NOTE: For other platforms, do something about "#include <windows.h>" in
REM       the anaglyph_dll_bridge.cpp file. Until then, don't do other
REM       platforms. (I can't even test them anyways.)
REM       Of note, Anaglyph only has Windows and macOS downloads.
@echo [%time%] Finished