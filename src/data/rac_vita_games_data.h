// rac_vita_games_data.h - CORRECTED (Gravity Bomb removed)
// RC2 Going Commando has 20 weapons (not 21)

#ifndef RAC_VITA_GAMES_DATA_H
#define RAC_VITA_GAMES_DATA_H

#include <vector>
#include <string>
#include <cstdint>

enum class GameType {
    UNKNOWN = 0,
    RAC1_VITA,
    RAC2_VITA,
    RAC3_VITA
};

struct GameValue {
    std::string name;
    std::string description;
    uint32_t offset;
    int32_t min_value;
    int32_t max_value;
    uint32_t byte_size;
};

struct GameWeapon {
    std::string name;
    std::string description;
    uint32_t ammo_offset;
    int32_t min_ammo;
    int32_t max_ammo;
    uint32_t byte_size;
};

struct GameGadget {
    std::string name;
    std::string description;
    uint32_t offset;
    uint8_t bit_index;
};

struct GameUnlockable {
    std::string name;
    std::string description;
    uint32_t offset;
    uint8_t bit_index;
};

// ============================================================================
// RATCHET & CLANK HD (VITA)
// ============================================================================

namespace RAC1_VITA_DATA {
    inline const std::vector<GameValue> VALUES = {
        {"Bolts", "Current bolt count", 36, 0, 999999, 4},
    };

    inline const std::vector<GameWeapon> WEAPONS = {
        {"Bomb Glove", "Starting weapon", 324, 0, 40, 4},
        {"Pyrocitor", "Flamethrower", 348, 0, 240, 4},
        {"Blaster", "Rapid fire pistol", 344, 0, 200, 4},
        {"Devastator", "Rocket launcher", 328, 0, 20, 4},
        {"Visibomb Gun", "Guided missiles", 336, 0, 20, 4},
        {"Mine Glove", "Proximity mines", 352, 0, 50, 4},
        {"Tesla Claw", "Electric weapon", 360, 0, 240, 4},
        {"Glove of Doom", "Spawns agents", 364, 0, 10, 4},
        {"RYNO", "Ultimate weapon", 376, 0, 50, 4},
        {"Drone Device", "Attack drones", 380, 0, 10, 4},
        {"Decoy Glove", "Spawns decoy", 384, 0, 20, 4},
    };

    inline const std::vector<GameGadget> GADGETS = {
        {"Heli-Pack", "Helicopter backpack", 442, 0},
        {"Thruster-Pack", "Jetpack", 443, 0},
        {"Hydro-Pack", "Underwater propulsion", 444, 0},
        {"Sonic Summoner", "Calls transport", 445, 0},
        {"O2 Mask", "Underwater breathing", 446, 0},
        {"Pilot Helmet", "Fly ships", 447, 0},
        {"Swingshot", "Grappling hook", 452, 0},
        {"Hydrodisplacer", "Water management", 462, 0},
        {"Trespasser", "Hacking tool", 466, 0},
        {"Metal Detector", "Find items", 467, 0},
        {"Magneboots", "Magnetic boots", 468, 0},
        {"Grindboots", "Rail grinding", 469, 0},
        {"Hoverboard", "Hoverboard", 470, 0},
        {"Hologuise", "Disguise", 471, 0},
        {"Gadgetron PDA", "PDA device", 472, 0},
        {"Map-o-Matic", "Map display", 473, 0},
        {"Bolt Grabber", "Magnetic bolts", 474, 0},
        {"Persuader", "Unlock doors", 475, 0},
    };

    inline const std::vector<GameUnlockable> UNLOCKABLES = {
        {"Suck Cannon Owned", "Unlock Suck Cannon", 449, 0},
        {"Bomb Glove Owned", "Unlock Bomb Glove", 450, 0},
        {"Devastator Owned", "Unlock Devastator", 451, 0},
        {"Visibomb Gun Owned", "Unlock Visibomb Gun", 453, 0},
        {"Taunter Owned", "Unlock Taunter", 454, 0},
        {"Blaster Owned", "Unlock Blaster", 455, 0},
        {"Pyrocitor Owned", "Unlock Pyrocitor", 456, 0},
        {"Mine Glove Owned", "Unlock Mine Glove", 457, 0},
        {"Walloper Owned", "Unlock Walloper", 458, 0},
        {"Tesla Claw Owned", "Unlock Tesla Claw", 459, 0},
        {"Glove of Doom Owned", "Unlock Glove of Doom", 460, 0},
        {"Morph-o-Ray Owned", "Unlock Morph-o-Ray", 461, 0},
        {"RYNO Owned", "Unlock RYNO", 463, 0},
        {"Drone Device Owned", "Unlock Drone Device", 464, 0},
        {"Decoy Glove Owned", "Unlock Decoy Glove", 465, 0},
    };
}

// ============================================================================
// RATCHET & CLANK 2: GOING COMMANDO HD (VITA)
// CORRECTED: 20 weapons total (Gravity Bomb removed - doesn't exist!)
// ============================================================================

namespace RAC2_VITA_DATA {
    inline const std::vector<GameValue> VALUES = {
        {"Bolts", "Current bolt count", 36, 0, 9999999, 4},
        {"Raritanium", "Upgrade currency", 40, 0, 99999, 4},
    };

    // 21 WEAPONS TOTAL - Corrected with verified Vita offsets + Ultra max ammo
    inline const std::vector<GameWeapon> WEAPONS = {
        // Clank weapon
        {"Clank Zapper", "Clank's weapon", 460, 0, 30, 4},  // VERIFIED
        
        // LEGACY WEAPONS (from RC1) - CORRECTED OFFSETS + Ultra max ammo
        {"Bomb Glove", "From RC1", 472, 0, 60, 4},          // VERIFIED 
        {"Visibomb Gun", "From RC1", 480, 0, 30, 4},        // VERIFIED 
        {"Decoy Glove", "From RC1", 492, 0, 30, 4},         // VERIFIED 
        {"Tesla Claw", "From RC1", 496, 0, 400, 4},         // VERIFIED 
        
        // NEW MEGACORP WEAPONS - Ultra max ammo values
        {"Chopper", "Throwing blades", 512, 0, 70, 4},      // Ultra Multi Star
        {"Pulse Rifle", "Energy weapon", 516, 0, 20, 4},    // Ultra Vaporizer
        {"Seeker Gun", "Homing missiles", 520, 0, 50, 4},   // Ultra HK22
        {"Hoverbomb Gun", "Guided bombs", 524, 0, 25, 4},   // Ultra Tetrabomb
        {"Blitz Gun", "Lightning weapon", 528, 0, 60, 4},   // Ultra Blitz Cannon
        {"Minirocket Tube", "Mini rockets", 532, 0, 60, 4}, // Ultra Mega Rocket Cannon
        {"Plasma Coil", "Plasma weapon", 536, 0, 50, 4},    // Ultra Plasma Storm
        {"Lava Gun", "Lava launcher", 540, 0, 400, 4},      // Ultra Meteor Gun
        {"Lancer", "Basic pistol", 544, 0, 500, 4},         // Ultra Heavy Lancer
        {"Synthenoid", "Spawns allies", 548, 0, 40, 4},     // Ultra Kilinoids (yes, 40!)
        {"Spiderbot Glove", "Spider bots", 552, 0, 16, 4},  // Mega Tankbot
        {"Bouncer", "Bomb launcher", 572, 0, 60, 4},        // Ultra Heavy Bouncer
        {"Miniturret Glove", "Spawns turrets", 588, 0, 50, 4}, // Ultra Megaturret
        {"Gravity Bomb", "Gravity weapon (→Mini Nuke)", 592, 0, 30, 4}, // Ultra Mini Nuke
        {"Zodiac", "Ultimate weapon", 596, 0, 4, 4},        // No upgrades
        {"RYNO II", "Super weapon", 600, 0, 100, 4},        // No upgrades
        {"Shield Charger", "Protective shield", 604, 0, 8, 4}, // Tesla Barrier
    };

    inline const std::vector<GameGadget> GADGETS = {
        {"Heli-Pack", "Helicopter pack", 658, 0},
        {"Thruster-Pack", "Jetpack", 659, 0},
        {"Hydro-Pack", "Water propulsion", 660, 0},
        {"Mapper", "Map display", 661, 0},
        {"Levitator", "Gravity platform", 664, 0},
        {"Swingshot", "Grappling hook", 669, 0},
        {"Gravity Boots", "Wall walking", 675, 0},
        {"Grindboots", "Rail grinding", 676, 0},
        {"Glider", "Glider", 677, 0},
        {"Dynamo", "Powers machinery", 692, 0},
        {"Electrolyzer", "Electric tool", 694, 0},
        {"Thermanator", "Heat visor", 695, 0},
        {"Tractor Beam", "Move objects", 702, 0},
        {"Biker Helmet", "Helmet", 704, 0},
        {"Quark Statuette", "Collectible", 705, 0},
        {"Box Breaker", "Stronger wrench", 706, 0},
        {"Infiltrator", "Hacking tool", 708, 0},
        {"Charge Boots", "Speed boots", 710, 0},
        {"Hypnomatic", "Mind control", 711, 0},
    };

    inline const std::vector<GameUnlockable> UNLOCKABLES = {
        {"Clank Zapper Owned", "Unlock Clank Zapper", 665, 0},
        {"Bomb Glove Owned", "Unlock Bomb Glove", 668, 0},
        {"Visibomb Gun Owned", "Unlock Visibomb Gun", 670, 0},
        {"Sheepinator Owned", "Unlock Sheepinator", 672, 0},
        {"Decoy Glove Owned", "Unlock Decoy Glove", 673, 0},
        {"Tesla Claw Owned", "Unlock Tesla Claw", 674, 0},
        {"Chopper Owned", "Unlock Chopper", 677, 0},
        {"Pulse Rifle Owned", "Unlock Pulse Rifle", 679, 0},
        {"Seeker Gun Owned", "Unlock Seeker Gun", 680, 0},
        {"Hoverbomb Gun Owned", "Unlock Hoverbomb Gun", 681, 0},
        {"Blitz Gun Owned", "Unlock Blitz Gun", 682, 0},
        {"Minirocket Tube Owned", "Unlock Minirocket Tube", 683, 0},
        {"Plasma Coil Owned", "Unlock Plasma Coil", 684, 0},
        {"Lava Gun Owned", "Unlock Lava Gun", 685, 0},
        {"Lancer Owned", "Unlock Lancer", 686, 0},
        {"Synthenoid Owned", "Unlock Synthenoid", 687, 0},
        {"Spiderbot Glove Owned", "Unlock Spiderbot Glove", 688, 0},
        {"Bouncer Owned", "Unlock Bouncer", 693, 0},
        {"Miniturret Glove Owned", "Unlock Miniturret Glove", 697, 0},
        {"Zodiac Owned", "Unlock Zodiac", 699, 0},
        {"RYNO II Owned", "Unlock RYNO II", 700, 0},
        {"Shield Charger Owned", "Unlock Shield Charger", 701, 0},
        {"Walloper Owned", "Unlock Walloper", 709, 0},
    };
}

// ============================================================================
// RATCHET & CLANK 3: UP YOUR ARSENAL HD (VITA)
// ============================================================================

namespace RAC3_VITA_DATA {
    inline const std::vector<GameValue> VALUES = {
        {"Bolts", "Current bolt count", 36, 0, 9999999, 4},
        {"Max All Weapon EXP", "Set all weapons to max level", 0xFFFFFFFF, 0, 0, 0},
    };

    // 20 WEAPONS TOTAL - 14 native RC3 + 6 RC2 unlockables
    inline const std::vector<GameWeapon> WEAPONS = {
        // Native RC3 weapons (from uya.json)
        {"Shock Blaster", "Electric pistol", 716, 0, 60, 4},        // Omega Shock Cannon
        {"Nitro Launcher", "Rocket launcher", 1036, 0, 16, 4},      // Omega Nitro Eruptor
        {"N60 Storm", "Machine gun", 748, 0, 400, 4},               // Omega N90 Hurricane
        {"Plasma Whip", "Melee weapon", 1068, 0, 60, 4},            // Omega Quantum Whip
        {"Infector", "Infection weapon", 780, 0, 30, 4},            // Omega Infecto-Bomb
        {"Suck Cannon", "Vortex weapon", 0, 0, 30, 4},              // Omega Vortex Cannon (NO AMMO - offset unknown)
        {"Spitting Hydra", "Multi-rocket", 844, 0, 30, 4},          // Omega Tempest
        {"Agents of Doom", "Spawns agents", 908, 0, 12, 4},         // Omega Agents of Dread
        {"Flux Rifle", "Sniper rifle", 1004, 0, 20, 4},             // Omega Splitter Rifle
        {"Annihilator", "Laser weapon", 812, 0, 30, 4},             // Omega Decimator
        {"Holoshield Glove", "Shield generator", 972, 0, 14, 4},    // Omega Ultrashield Launcher
        {"Disc Blade Gun", "Disc launcher", 876, 0, 30, 4},         // Omega Multi-Disc Gun
        {"Rift Inducer", "Black hole gun", 940, 0, 16, 4},          // Omega Rift Ripper
        {"Qwack-O-Ray", "Transform weapon", 1132, 0, 0, 4},         // Omega Qwack-O-Blitzer (Infinite)
        
        // RC2 weapons (unlockable in RC3) - VERIFIED OFFSETS ✅
        {"Miniturret Glove", "From RC2", 644, 0, 16, 4},            // Omega Megaturret 
        {"Lava Gun", "From RC2", 628, 0, 300, 4},                   // Omega Meteor Gun 
        {"Bouncer", "From RC2", 636, 0, 16, 4},                     // Omega Heavy Bouncer 
        {"Plasma Coil", "From RC2", 624, 0, 25, 4},                 // Omega Plasma Storm 
        {"Shield Charger", "From RC2", 648, 0, 5, 4},               // Omega Tesla Barrier 
        
        {"RY3NO", "Ultimate weapon", 1164, 0, 50, 4},               // RYNOCIRATOR V5
    };

    // R&C3 weapon EXP system - 20 total (14 RC3 + 6 RC2)
    inline const std::vector<GameValue> WEAPON_EXP = {
        // Native RC3 weapons
        {"Shock Blaster EXP", "Weapon experience", 1676, 0, 999999, 4},
        {"Nitro Launcher EXP", "Weapon experience", 1996, 0, 999999, 4},
        {"N60 Storm EXP", "Weapon experience", 1708, 0, 999999, 4},
        {"Plasma Whip EXP", "Weapon experience", 2028, 0, 999999, 4},
        {"Infector EXP", "Weapon experience", 1740, 0, 999999, 4},
        {"Qwack-O-Ray EXP", "Weapon experience", 2092, 0, 999999, 4},          
        {"Spitting Hydra EXP", "Weapon experience", 2060, 0, 999999, 4},
        {"Agents of Doom EXP", "Weapon experience", 1868, 0, 999999, 4},
        {"Flux Rifle EXP", "Weapon experience", 1964, 0, 999999, 4},
        {"Annihilator EXP", "Weapon experience", 1772, 0, 999999, 4},
        {"Holoshield EXP", "Weapon experience", 1932, 0, 999999, 4},
        {"Disc Blade Gun EXP", "Weapon experience", 1836, 0, 999999, 4},
        {"Rift Inducer EXP", "Weapon experience", 1900, 0, 999999, 4},
        {"Suck Cannon EXP", "Weapon experience", 1804, 0, 999999, 4},
        
        // RC2 weapons - VERIFIED OFFSETS ✅
        {"Miniturret EXP", "RC2 weapon", 1604, 0, 999999, 4},               
        {"Lava Gun EXP", "RC2 weapon", 1588, 0, 999999, 4},                
        {"Bouncer EXP", "RC2 weapon", 1596, 0, 999999, 4},                  
        {"Plasma Coil EXP", "RC2 weapon", 1584, 0, 2700000, 4},             
        {"Shield Charger EXP", "RC2 weapon", 1608, 0, 999999, 4},          
        
        {"RY3NO EXP", "Weapon experience", 2124, 0, 5700000, 4},
    };

    inline const std::vector<GameGadget> GADGETS = {
        {"Heli Pack", "Helicopter pack", 1194, 0},
        {"Thruster Pack", "Jetpack", 1195, 0},
        {"Hydro Pack", "Water propulsion", 1196, 0},
        {"Map-o-Matic", "Map display", 1197, 0},
        {"Commando Suit", "Armor", 1198, 0},
        {"Bolt Grabber v2", "Enhanced bolt magnet", 1199, 0},
        {"Levitator", "Gravity platform", 1200, 0},
        {"Omniwrench", "Upgraded wrench", 1201, 0},
        {"Hypershot", "Long grapple", 1203, 0},
        {"Gravity Boots", "Wall walking", 1205, 0},
        {"Refractor", "Deflects lasers", 1210, 0},
        {"The Hacker", "Computer hacking", 1212, 0},
        {"Charge Boots", "Speed boots", 1221, 0},
        {"Tyhrraguise", "Disguise", 1222, 0},
        {"Warp Pad", "Teleporter", 1223, 0},
        {"Nano Pak", "Health upgrade", 1224, 0},
        {"Star Map", "Galactic map", 1226, 0},
        {"Master Plan", "Story item", 1227, 0},
        {"PDA", "Device", 1228, 0},
    };

    inline const std::vector<GameUnlockable> UNLOCKABLES = {
        {"Shock Blaster V1 Owned", "Unlock Shock Blaster", 1231, 0},
        {"N60 Storm V1 Owned", "Unlock N60 Storm", 1239, 0},
        {"Infector V1 Owned", "Unlock Infector", 1247, 0},
        {"Annihilator V1 Owned", "Unlock Annihilator", 1255, 0},
        {"Spitting Hydra V1 Owned", "Unlock Spitting Hydra", 1263, 0},
        {"Disc Blade Gun V1 Owned", "Unlock Disc Blade Gun", 1271, 0},
        {"Agents of Doom V1 Owned", "Unlock Agents of Doom", 1279, 0},
        {"Rift Inducer V1 Owned", "Unlock Rift Inducer", 1287, 0},
        {"Holoshield V1 Owned", "Unlock Holoshield", 1295, 0},
        {"Flux Rifle V1 Owned", "Unlock Flux Rifle", 1303, 0},
        {"Nitro Launcher V1 Owned", "Unlock Nitro Launcher", 1311, 0},
        {"Plasma Whip V1 Owned", "Unlock Plasma Whip", 1319, 0},
        {"Suck Cannon V1 Owned", "Unlock Suck Cannon", 1327, 0},
		{"Qwack-O-Ray V1 Owned", "Unlock Qwack-O-Ray", 1335, 0},
        {"RY3NO V1 Owned", "Unlock RY3NO", 1343, 0},
        {"Plasma Coil Owned", "Unlock Plasma Coil (RC2)", 1208, 0},
        {"Lava Gun Owned", "Unlock Lava Gun (RC2)", 1209, 0},
        {"Bouncer Owned", "Unlock Bouncer (RC2)", 1211, 0},
        {"Miniturret Owned", "Unlock Miniturret (RC2)", 1213, 0},
        {"Shield Charger Owned", "Unlock Shield Charger (RC2)", 1214, 0},
    };
}

// ============================================================================
// GAME DATA WRAPPER
// ============================================================================

struct GameData {
    GameType type;
    std::string name;
    std::vector<GameValue> values;
    std::vector<GameWeapon> weapons;
    std::vector<GameGadget> gadgets;
    std::vector<GameUnlockable> unlockables;
    std::vector<GameValue> extra_values;
    
    GameData() : type(GameType::UNKNOWN), name("Unknown Game") {}
    
    GameData(GameType t) : type(t) {
        switch(t) {
            case GameType::RAC1_VITA:
                name = "Ratchet & Clank HD";
                values = RAC1_VITA_DATA::VALUES;
                weapons = RAC1_VITA_DATA::WEAPONS;
                gadgets = RAC1_VITA_DATA::GADGETS;
                unlockables = RAC1_VITA_DATA::UNLOCKABLES;
                break;
            case GameType::RAC2_VITA:
                name = "Ratchet & Clank 2 HD";
                values = RAC2_VITA_DATA::VALUES;
                weapons = RAC2_VITA_DATA::WEAPONS;
                gadgets = RAC2_VITA_DATA::GADGETS;
                unlockables = RAC2_VITA_DATA::UNLOCKABLES;
                break;
            case GameType::RAC3_VITA:
                name = "Ratchet & Clank 3 HD";
                values = RAC3_VITA_DATA::VALUES;
                weapons = RAC3_VITA_DATA::WEAPONS;
                gadgets = RAC3_VITA_DATA::GADGETS;
                unlockables = RAC3_VITA_DATA::UNLOCKABLES;
                extra_values = RAC3_VITA_DATA::WEAPON_EXP;
                break;
            default:
                break;
        }
    }
};

inline GameType DetectGameType(const std::vector<uint8_t>& saveData) {
    size_t fileSize = saveData.size();
    
    if (fileSize > 0) {
        if (fileSize >= 650000 && fileSize <= 850000) {
            return GameType::RAC1_VITA;
        }
        else if (fileSize >= 850000 && fileSize <= 1150000) {
            return GameType::RAC2_VITA;
        }
        else if (fileSize >= 1150000) {
            return GameType::RAC3_VITA;
        }
    }
    
    return GameType::UNKNOWN;
}

inline std::string GetGameName(GameType type) {
    switch(type) {
        case GameType::RAC1_VITA: return "Ratchet & Clank HD";
        case GameType::RAC2_VITA: return "Ratchet & Clank 2 HD";
        case GameType::RAC3_VITA: return "Ratchet & Clank 3 HD";
        default: return "Unknown Game";
    }
}

#endif // RAC_VITA_GAMES_DATA_H