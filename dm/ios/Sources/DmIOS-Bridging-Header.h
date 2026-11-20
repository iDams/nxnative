// The C and Objective-C functions Launcher.swift, GameMenu.swift and LauncherSettings.swift call (CloudSaves.h, DmGameMenu.h, mods_api.h's mods). project.yml points SWIFT_OBJC_BRIDGING_HEADER here.
#import "CloudSaves.h"
#import "DmGameMenu.h"
#include "mods_api.h"   // Mods.swift: the per-mod check and the import (hle_mods.c)
