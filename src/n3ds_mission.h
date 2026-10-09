// Spear of Destiny mission picker for 3DS.
//
// On PC the mission packs are chosen with --mission, which a .cia cannot
// pass. When the data folder holds more than one mission, this shows a
// short text menu on the top screen before the game starts.

#ifndef N3DS_MISSION_H
#define N3DS_MISSION_H

int N3DS_ChooseSpearMission (void);     // value for param_mission (0-3)

#endif
