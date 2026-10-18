#pragma once

#include <strug/controls/StrugInputAction.h>

#include <glade/controls/Key.h>
#include <glade/controls/KeyMapper.h>

class StrugKeyMapper: public Glade::KeyMapper
{
public:
  StrugKeyMapper()
  {
    // This is for WalkingTheWorld state only
    remap(Glade::Key::GLADE_KEY_W, StrugInputAction::CHARACTER_MOVE_FORWARD);
    remap(Glade::Key::GLADE_KEY_S, StrugInputAction::CHARACTER_MOVE_BACKWARDS);
    remap(Glade::Key::GLADE_KEY_A, StrugInputAction::CHARACTER_STRAFE_LEFT);
    remap(Glade::Key::GLADE_KEY_D, StrugInputAction::CHARACTER_STRAFE_RIGHT);
    remap(Glade::Key::GLADE_KEY_Q, StrugInputAction::CHARACTER_TURN_LEFT);
    remap(Glade::Key::GLADE_KEY_E, StrugInputAction::CHARACTER_TURN_RIGHT);
    remap(Glade::Key::GLADE_KEY_SPACE, StrugInputAction::CHARACTER_FLY_UP);
    remap(Glade::Key::GLADE_KEY_X, StrugInputAction::CHARACTER_FLY_DOWN);

    remap(Glade::Key::GLADE_KEY_UP, StrugInputAction::FREE_CAMERA_MOVE_FORWARD);
    remap(Glade::Key::GLADE_KEY_DOWN, StrugInputAction::FREE_CAMERA_MOVE_BACKWARDS);
    remap(Glade::Key::GLADE_KEY_LEFT, StrugInputAction::FREE_CAMERA_STRAFE_LEFT);
    remap(Glade::Key::GLADE_KEY_RIGHT, StrugInputAction::FREE_CAMERA_STRAFE_RIGHT);
    remap(Glade::Key::GLADE_KEY_O, StrugInputAction::FREE_CAMERA_FLY_UP);
    remap(Glade::Key::GLADE_KEY_L, StrugInputAction::FREE_CAMERA_FLY_DOWN);

    remap(Glade::Key::GLADE_KEY_1, StrugInputAction::TOGGLE_CHARACTER_FLY_MODE);
    remap(Glade::Key::GLADE_KEY_2, StrugInputAction::TOGGLE_TERRAIN_COLLISIONS);
    remap(Glade::Key::GLADE_KEY_3, StrugInputAction::RESET_CAMERA_AND_CHARACTER_POSITIONS);
    remap(Glade::Key::GLADE_KEY_4, StrugInputAction::SWITCH_CAMERA_MODE);
    remap(Glade::Key::GLADE_KEY_5, StrugInputAction::TOGGLE_COLLIDERS_VISUALISATION);
  }
};

