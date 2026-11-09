@echo off
title Sonic Free Riders - PC Keyboard and Mouse Edition (by @mrunemployed)
cd /d "%~dp0"

echo ========================================================================
echo         SONIC FREE RIDERS - PC KEYBOARD ^& MOUSE MOD v1.0
echo                       Created by @mrunemployed
echo ========================================================================
echo   Controls: Dual Mode (Keyboard + Mouse OR Keyboard Only)
echo   Custom Keybindings: Configurable in sonic_kb_mouse.ini
echo   Toggle Mode Anytime: Press [F9] in-game (chime sounds upon toggle)
echo   Toggle Mouse Lock  : Press [F8] in-game
echo ------------------------------------------------------------------------
echo   CLASSIC RIDERS DRIFT SYSTEM:
echo     [Q] or [Mouse 4] or [Ctrl+A] : DRIFT LEFT  (Sharp Carve + Boost Exit)
echo     [E] or [Mouse 5] or [Ctrl+D] : DRIFT RIGHT (Sharp Carve + Boost Exit)
echo     * Releasing Drift at corner exit triggers a speed surge!
echo ------------------------------------------------------------------------
echo   IN-GAME RACING:
echo     Steer Left / Right : [A] / [D] or [Left] / [Right] Arrow or Move Mouse
echo     Lean Forward (Tuck): [W] or [Up] Arrow (Speed acceleration)
echo     Lean Back (Brake)  : [S] or [Down] Arrow (Brake / Weight shift)
echo     Crouch / Jump      : Hold [Space] / [Z] / [Left Click], release to Jump
echo     Kick Dash / Boost  : Hold [Shift] / [X] / [Right Click], release to Boost
echo     Use / Shake Item   : [F] or [R] or [Middle Click] (Scroll Wheel Click)
echo     Wipe Screen        : Hold [F]/[R]/[Middle Click] + move Mouse or [WASD]
echo     Tag Mode Hold Hands: [T] or [Left Alt]
echo     Swap Relay Racer   : [Tab]
echo     Pause Game         : [P]
echo ------------------------------------------------------------------------
echo   MENUS / DIALOGS / CONFIRM / CANCEL:
echo     Confirm / Select   : [Enter] or [Space] or [Left Click] or [Z]
echo                          (Instantly confirms ANY dialog prompt)
echo     Cancel / Back      : [Escape] or [Backspace] or [C]
echo     Move Menu Cursor   : Move Mouse (smooth snappy servo) OR [Arrows] / [WASD]
echo     Rotate 3D Ring     : [Mouse Wheel] OR [1] / [2] OR [Left] / [Right] Arrow
echo     Special Options    : [3] / [4] OR [Up] / [Down] Arrow
echo     Reset Cursor       : [Tab]
echo     Toggle Cursor      : [Y] or [V]
echo ------------------------------------------------------------------------
echo   HOTKEYS:
echo     [F8] Toggle Mouse Lock (locks cursor inside window / unlocks to desktop)
echo     [F9] Toggle Input Mode (Keyboard+Mouse vs 100%% Keyboard-Only)
echo ========================================================================
echo.
echo Launching Sonic Free Riders...
start "" "xenia_canary.exe" "default-DLLLoaderBase.xex"
exit
