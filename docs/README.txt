================================================================================
         SONIC FREE RIDERS - PC KEYBOARD & MOUSE MOD v1.0
                 (Classic Sonic Riders Handling Edition)
                       Created by @mrunemployed
================================================================================
Game: Sonic Free Riders (Xbox 360 on Xenia Canary)
Author: @mrunemployed
Platform: PC (Windows 10 / 11)

--------------------------------------------------------------------------------
1. ABOUT THIS MOD
--------------------------------------------------------------------------------
Sonic Free Riders was originally an Xbox 360 Kinect exclusive with no native
gamepad or keyboard controls. This standalone input enhancement mod by
@mrunemployed transforms the game into a fast, fluid PC racing experience:

  * NATIVE KEYBOARD & MOUSE SUPPORT: Full analog steering and responsive input.
  * CLASSIC SONIC RIDERS 1 & 2 DRIFTING: Dedicated Drift Left [Q] and Drift
    Right [E] keys that tightly carve around hairpin corners, with an instant
    speed surge boost upon exiting the drift!
  * SNAPPY MENU CURSOR SERVO: Closed-loop mouse servo glides effortlessly
    across menus to Confirm/Cancel without getting sluggish or stuck.
  * UNIVERSAL DIALOG CONFIRM / CANCEL: One-press Enter/Space/Left Click to
    confirm any prompt; Escape/Backspace/C to cancel.
  * FULL CUSTOM KEY BINDINGS: Edit "sonic_kb_mouse.ini" to bind ANY keys you
    prefer (WASD, Arrows, Numpad, Mouse buttons, etc.).
  * DUAL INPUT MODES: Seamlessly switch between Keyboard+Mouse and 100%
    Keyboard-Only mode by pressing [F9] anytime.
  * CLEAN, NON-OVERLAPPING ARCHITECTURE: Works purely via a drop-in XInput
    driver wrapper (xinput1_4.dll). Does NOT alter or redistribute any game
    code or third-party patches.

--------------------------------------------------------------------------------
2. QUICK START / INSTALLATION
--------------------------------------------------------------------------------
1. Place "xinput1_4.dll", "xinput1_3.dll", and "sonic_kb_mouse.ini" into your
   Sonic Free Riders (Xenia Canary) root folder.
2. Launch the game using "Play_Sonic_Free_Riders.bat" (or run Xenia directly).
3. Play with Keyboard & Mouse or pure Keyboard!

--------------------------------------------------------------------------------
3. DEFAULT CONTROLS
--------------------------------------------------------------------------------
[RACING CONTROLS]
  * Steer Left / Right : [A] / [D] or [Left] / [Right] Arrow or Move Mouse
  * Lean Forward (Tuck): [W] or [Up Arrow] (Speed acceleration)
  * Lean Back (Brake)  : [S] or [Down Arrow] (Brake / Air pitch down)
  * Drift Left         : [Q] or [Mouse Side 1] or [Ctrl + A] (Sharp Carve + Boost)
  * Drift Right        : [E] or [Mouse Side 2] or [Ctrl + D] (Sharp Carve + Boost)
  * Crouch & Jump      : Hold [Space], [Z], or [Left Click]; release to jump!
  * Kick Dash / Boost  : Hold [Left Shift], [X], or [Right Click]; release to boost!
  * Use / Shake Item   : Tap [F], [R], or [Middle Click] to use; hold to shake
  * Screen Wipe        : Hold [F]/[R] + move Mouse or [WASD]
  * Power Punch        : Drifting automatically punches obstacles!
  * Tag Mode           : Hold [T] or [Left Alt] (Hold hands)
  * Pause Game         : [P] or [Pause/Break]

[MENUS & DIALOGS]
  * Confirm / Select   : [Enter], [Spacebar], [Left Click], or [Z]
  * Cancel / Back      : [Escape], [Backspace], or [C]
  * Move Menu Cursor   : Move Mouse OR [Arrow Keys] / [WASD] / [I, J, K, L]
  * Rotate 3D Ring     : [Mouse Scroll Wheel], [1] / [2], or [Left] / [Right] Arrow
  * Special Options    : [3] / [4] or [Up] / [Down] Arrow
  * Reset Cursor       : [Tab]
  * Toggle Cursor      : [Y] or [V]

[HOTKEYS]
  * [F8] : Toggle Mouse Lock (locks cursor inside window / unlocks to desktop)
  * [F9] : Toggle Input Mode (Keyboard+Mouse Mode vs 100% Keyboard-Only Mode)
  (Hotkeys are non-overlapping with Xenia and can be changed in sonic_kb_mouse.ini)

--------------------------------------------------------------------------------
4. UNOBVIOUS KINECT GESTURES TRANSLATED TO PC (HOW TO PASS TRICKY MISSIONS)
--------------------------------------------------------------------------------
Sonic Free Riders had several special Kinect physical body motions that can be
confusing when playing on PC. Here is how every unintuitive mechanic works:

1. FLYING & SKYDIVING (MISSION 12 & RING FLIGHT SECTIONS)
   ------------------------------------------------------
   * Original Kinect Motion: Spreading arms wide like wings, tilting body
     forward/backward to pitch and side-to-side to bank through ring hoops.
   * PC Keyboard/Mouse Controls:
     - CLIMB / PITCH UP   : HOLD [Spacebar] (or Left Click / Controller [A]).
                            Keep holding to climb into high rings!
     - DIVE / PITCH DOWN  : RELEASE [Spacebar].
                            Letting go will dive your character downward.
     - STEER LEFT / RIGHT : [A] / [D] or move Mouse Left / Right.
     - ALTITUDE FLAP      : Tap [W] (or Mouse Up) to flap/surge upward.
     - AIR BRAKE / SLOW   : Tap [S] (or Mouse Down) to stabilize descent.
   * PRO-TIP FOR MISSION 12: Hold [Spacebar] to rise up to upper ring lines,
     release it when the rings slope down, and steer with [A]/[D] to glide
     cleanly through the centers.

2. SWIMMING (UNDERWATER TUNNELS & WATER RAPIDS)
   --------------------------------------------
   * Original Kinect Motion: Doing physical breaststroke / paddle motions with
     both hands to swim through submerged caverns.
   * PC Keyboard/Mouse Controls:
     - Hold [W] / [Up Arrow] for forward propulsion.
     - Rapidly tap [Shift] or [Spacebar] (or alternate [A] and [D]) to perform
       continuous swimming strokes and accelerate through currents.
     - Steer direction using Mouse or [A] / [D].

3. SCREEN WIPE (INK SQUID & MUD SPLATTERS)
   ---------------------------------------
   * Original Kinect Motion: Waving your hand back and forth across your face
     to wipe away black squid ink or blinding mud.
   * PC Keyboard/Mouse Controls:
     - Hold [F] / [R] or [Middle Click] and shake your Mouse rapidly left
       and right (or alternate [A] and [D] quickly) to instantly wipe the
       screen clean and restore full visibility.

4. POWER PUNCH (SMASHING BARRICADES & ROCK PILLARS)
   -------------------------------------------------
   * Original Kinect Motion: Throwing a real punch to the left or right to
     destroy track barricades, stone blocks, and giant obstacles.
   * PC Keyboard/Mouse Controls:
     - Press [Q] (Left Drift / Punch) or [E] (Right Drift / Punch) when
       approaching an obstacle on that side.
     - You can also tap [F] or [R] (Item button) to trigger a direct punch.
     - The mod automatically deflects and smashes obstacles upon contact!

5. TRICK JUMPS & CHARGING (RAMP LIPS & S-RANK STUNTS)
   ---------------------------------------------------
   * Original Kinect Motion: Squatting down low to charge energy, then jumping
     straight up into the air the instant you reach the tip of the ramp.
   * PC Keyboard/Mouse Controls:
     - HOLD [Spacebar], [Z], or [Left Click] BEFORE reaching the ramp lip.
       You will see your character crouch down and charge jump power.
     - RELEASE [Spacebar] EXACTLY as your board reaches the red/yellow edge
       at the top of the ramp to launch sky-high and automatically score
       X or S rank aerial trick combos!

6. KICK DASHING (ACCELERATION & STARTING LINE)
   --------------------------------------------
   * Original Kinect Motion: Kicking your leg backward like pushing a skateboard.
   * PC Keyboard/Mouse Controls:
     - Tap [Left Shift], [X], or [Right Click] repeatedly to kick-dash off
       the start line or regain top speed after hitting a wall or taking damage.

7. MENU NAVIGATION & DIALOG POPUPS
   --------------------------------
   * Original Kinect Motion: Hovering an open hand cursor over a floating orb
     and holding still to select.
   * PC Keyboard/Mouse Controls:
     - Simply move your Mouse! The built-in servo moves the cursor instantly.
     - Press [Enter], [Spacebar], or [Left Click] to CONFIRM any dialog.
     - Press [Escape], [Backspace], or [C] to CANCEL or go back.

--------------------------------------------------------------------------------
5. CUSTOM KEYBINDING IN "sonic_kb_mouse.ini"
--------------------------------------------------------------------------------
Open "sonic_kb_mouse.ini" in any text editor to customize any key:
  Example:
    steer_left   = A, LEFT
    steer_right  = D, RIGHT
    drift_left   = Q, MOUSE_4
    drift_right  = E, MOUSE_5
    jump_crouch  = SPACE, Z, MOUSE_LEFT
    kick_dash    = LSHIFT, X, MOUSE_RIGHT

--------------------------------------------------------------------------------
6. CREDITS & LEGAL
--------------------------------------------------------------------------------
Created by @mrunemployed.
Sonic the Hedgehog and Sonic Free Riders are copyright (C) SEGA.
This is a non-commercial PC fan tool / input driver.
================================================================================
