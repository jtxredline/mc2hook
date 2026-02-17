#include "discord_rpc.h"
#include <discord-rpc/gamesdk/discord.h> // SDK
#include <chrono>
#include <age/mcraceconfig/raceconfig.h>
#include <age/managers/netmanager.h>
#include <age/vehicle/automgr.h>
#include <age/types.h>
#include <age/globals.h>
#include <age/string/string.h>
#include <age/math/math.h>

static const int64_t DISCORD_APP_ID = 1472018278317555722;
static discord::Core* g_core = nullptr;
static int64_t g_startTimestamp = 0;
static GameState g_lastState = GameState::Boot;

static int64_t GetUnixTimestamp()
{
    using namespace std::chrono;
    return duration_cast<seconds>(
        system_clock::now().time_since_epoch()
    ).count();
}

// Race names
struct RaceNameEntry
{
    const char* devName;
    const char* cleanName;
};

static const char* raceTypeNames[] =
{
    "Cruising:",
    "Cruising:",  // Finding hookman
    "Cruising:",
    "Cruising:",  // Following
    "Racing:",
    "Racing:",
    "Racing:",
    "Racing:",
    "Racing:",
    "Racing:",
    "CTF:",
    "Detonator:",
    "Lose The Cops:",
    "Destroy:"
};

static const RaceNameEntry raceNameTable[] =
{
    { "lamauro_arcade_circuit_one", "LA Turismo" },
    { "lachris_arcade_circuit_two", "River Raid" },
    { "lachris_arcade_circuit_three", "Runoff" },
    { "lachris_arcade_circuit_four", "Arrivals & Departures" },
    { "lachris_arcade_circuit_five", "Beverly Thrasher" },

    { "paris_arcade_circuit_one", "Freefall" },
    { "paris_arcade_circuit_two", "Louvre Launch" },
    { "paris_arcade_circuit_three", "Tourist Trap" },
    { "paris_arcade_circuit_four", "Arc de Triomphe" },
    { "paris_arcade_circuit_five", "Necro Modellista" },
    { "paris_arcade_circuit_six", "Evening Mass" },

    { "tokyoarcade_arcade_circuit_one", "Shinjuku Speedway" },
    { "tokyoarcade_arcade_circuit_two", "Ouroboros" },
    { "tokyowing_arcade_circuit_three", "Sonic Waves" },
    { "tokyoarcade_arcade_circuit_four", "Tribulation" },
    { "tokyoarcade_arcade_circuit_five", "Hangtime" },
    { "tokyotroy_arcade_circuit_six", "Clouded Vision" },
    { "tokyotroy_arcade_circuit_seven", "Bullet Train II" },

    { "lamauro_moses_checkpoint_one", "Initiation" },
    { "lamauro_moses_checkpoint_two", "Welcome to the Club" },
    { "lamauro_steven_checkpoint_one", "Takeoff Time" },
    { "lachrisbeta_steven_checkpoint_two", "Triple Threat" },
    { "lamauro_maria_checkpoint_one", "Freeway Rally" },
    { "lamauro_maria_checkpoint_two", "Corner Cutter" },
    { "lamauro_angel_checkpoint_one", "Hollywood Hysteria" },
    { "lamauro_angel_siderace", "Drag" },
    { "lamauro_angel_checkpoint_two", "Cheaters Never Win" },
    { "lachris_gina_checkpoint_one", "Unexpected Guests" },
    { "lachris_gina_siderace", "Showdown" },
    { "lachris_gina_bike_tutorial", "Weight Training" },
    { "lachris_gina_checkpoint_two", "Flight to the Finish" },
    { "lawing_hector_losethecops", "Lose The Cops" },
    { "lachrisbeta_hector_checkpoint_two", "Block Party" },
    { "lachrisbeta_hector_checkpoint_three", "Slipstream Chicanes" },
    { "lamaurobeta_dice_checkpoint_one", "Crossover" },
    { "lamaurobeta_dice_checkpoint_two", "Droppin' Knowledge" },

    { "parischris_blog_checkpoint_one", "Eiffel Visions" },
    { "parischris_blog_checkpoint_two", "Rooftop Vistas" },
    { "pariswing_julie_checkpoint_one", "Twin Turbo" },
    { "pariswing_julie_checkpoint_two", "Rollercoaster" },
    { "parischris_primo_checkpoint_one", "Seine Insane" },
    { "paris_primo_checkpoint_two", "Swerve" },
    { "parischris_stephane_checkpoint_one", "Roundabout Run" },
    { "parischris_stephane_checkpoint_two", "Over and Under" },
    { "paris_ian_checkpoint_one", "The Art of Racing" },
    { "paris_ian_checkpoint_two", "Few and Far Between" },
    { "paris_ian_siderace", "Monsoni Monsoon" },
    { "pariswing_farid_checkpoint_one", "Catacomb Criminal" },
    { "pariswing_farid_losethecops", "Lose The Cops" },
    { "pariswing_farid_siderace", "Extortion" },
    { "paris_parfait_checkpoint_one", "View From Above" },
    { "paris_parfait_checkpoint_two", "Haul" },

    { "tokyomauro_ricky_checkpoint_one", "Shibuya Scramble" },
    { "tokyomauro_ricky_checkpoint_two", "Ricky's Revenge" },
    { "tokyochris_shing_checkpoint_one", "Red Herring" },
    { "tokyochris_shing_siderace", "Ichiro's Challenge" },
    { "tokyochris_shing_checkpoint_two", "The Real Deal" },
    { "tokyo_haley_checkpoint_one", "Enigma" },
    { "tokyo_haley_checkpoint_two", "Intuition" },
    { "tokyomarc_nikko_checkpoint_one", "Drifter's Playground" },
    { "tokyomarc_nikko_checkpoint_two", "Savant of Speed" },
    { "tokyowing_zen_checkpoint_one", "Wisdom Gate" },
    { "tokyowing_zen_checkpoint_two", "Karma Rising" },
    { "tokyo_kenichi_checkpoint_one", "8-9-3" },
    { "tokyo_kenichi_checkpoint_two", "Machi-yakko" },
    { "tokyo_kenichi_checkpoint_three", "Oyabun" },
    { "tokyochris_makoto_checkpoint_one", "Rite of Passage" },
    { "tokyochris_makoto_checkpoint_two", "Enduro" },

    { "tokyomarc_worldchamptokyo_checkpoint_one", "Stage I - Intake" },
    { "tokyomarc_worldchamptokyo_checkpoint_two", "Stage II - Light" },
    { "paris_worldchampparis_checkpoint_one", "Stage III - Burn" },
    { "paris_worldchampparis_checkpoint_two", "Stage IV - Turbo" },
    { "lachrisbeta_worldchampla_checkpoint_one", "Stage V - Traction" },
    { "lawc_worldchampla_checkpoint_two", "Stage VI - Apex" }
};

static const int raceNameCount = sizeof(raceNameTable) / sizeof(raceNameTable[0]);

const char* GetCleanRaceName(const char* devName)
{
    if (!devName)
        return "Unknown Race";

    // To lower
    const char* lowerName = StringToLower(devName);

    for (int i = 0; i < raceNameCount; ++i)
    {
        if (std::strcmp(lowerName, raceNameTable[i].devName) == 0)
        {
            return raceNameTable[i].cleanName;
        }
    }

    // Fallback: return original name (mostly a custom race)
    return devName;
}

//

void Discord_Init()
{
    if (g_core)
        return;

    auto result = discord::Core::Create(
        DISCORD_APP_ID,
        DiscordCreateFlags_NoRequireDiscord,
        &g_core
    );

    if (result != discord::Result::Ok)
    {
        g_core = nullptr;
        return;
    }

    g_startTimestamp = GetUnixTimestamp();

    if (result != discord::Result::Ok)
    {
        printf("Discord init failed!\n");
    }
}

void Discord_Shutdown()
{
    if (g_core)
    {
        delete g_core;
        g_core = nullptr;
    }
}

void Discord_RunCallbacks()
{
    if (g_core)
        g_core->RunCallbacks();
}

void Discord_UpdateForState(GameState state)
{
    if (!g_core)
        return;

    // Avoid redundant updates
    if (state == g_lastState)
        return;

    g_lastState = state;

    discord::Activity activity{};

    const char* raceType;
    const char* raceName;
    const char* city;
    const char* vehicle;

    activity.SetType(discord::ActivityType::Playing);
    activity.GetTimestamps().SetStart(g_startTimestamp);

    activity.GetAssets().SetLargeImage("mc2_logo");
    activity.GetAssets().SetLargeText("Midnight Club 2");

    switch (state)
    {
    case GameState::MainMenu:
        activity.SetDetails("In main menu");
        break;

    case GameState::Loading:
        activity.SetDetails("Loading...");
        break;

    case GameState::Race:
    {
        char detailsText[256];
        raceType = raceTypeNames[mcRaceConfig::g_CurrentRaceType];
        vehicle = g_VehicleNames[mcRaceConfig::g_CurrentCar];
        city = g_CityNames[mcRaceConfig::g_CurrentCity];

        // Cruise/battle modes
        if (mcRaceConfig::g_CurrentRaceType == RACE_TYPE::roam             ||
            mcRaceConfig::g_CurrentRaceType == RACE_TYPE::find_hook        ||
            mcRaceConfig::g_CurrentRaceType == RACE_TYPE::hookman_cruise   ||
            mcRaceConfig::g_CurrentRaceType == RACE_TYPE::follow           ||
            mcRaceConfig::g_CurrentRaceType == RACE_TYPE::capture_the_flag ||
            mcRaceConfig::g_CurrentRaceType == RACE_TYPE::bomb_tag         ||
            mcRaceConfig::g_CurrentRaceType == RACE_TYPE::lose_the_cops    ||
            mcRaceConfig::g_CurrentRaceType == RACE_TYPE::destroy)
        {
            sprintf_s(detailsText, "%s %s - %s", raceType, city, vehicle);
        }

        // Any race mode
        if (mcRaceConfig::g_CurrentRaceType == RACE_TYPE::cp_unordered             ||
            mcRaceConfig::g_CurrentRaceType == RACE_TYPE::cp_unordered_time_local  ||
            mcRaceConfig::g_CurrentRaceType == RACE_TYPE::cp_unordered_time_global ||
            mcRaceConfig::g_CurrentRaceType == RACE_TYPE::cp_ordered               ||
            mcRaceConfig::g_CurrentRaceType == RACE_TYPE::cp_ordered_time_local    ||
            mcRaceConfig::g_CurrentRaceType == RACE_TYPE::cp_ordered_time_global)
        {
            raceName = GetCleanRaceName(mcRaceConfig::g_RaceName);
            sprintf_s(detailsText, "%s %s - %s", raceType, raceName, vehicle);
        }

        // Network state
        if (mcNetManager::IsNetworkMode == true)
        {
            char stateText[256];
            int numSlots = mcRaceConfig::g_NumPlayerSlots;
            int numPlayers = vehAutoMgr::Instance->m_NumSims;
            sprintf_s(stateText, "Multiplayer %d of %d", numPlayers, numSlots);
            activity.SetState(stateText);
        }

        activity.SetDetails(detailsText);

        // Select random image based on current city
        int min = 0;
        int max = 0;
        char imgName[256];
        sprintf_s(imgName, "mc2_logo"); // Default

        if (mcRaceConfig::g_CurrentCity == CITY::losangeles)
        {
            max = 10;
            int r = math::RandomInt(min, max);
            sprintf_s(imgName, "%s_%d", g_CityNamesDev[mcRaceConfig::g_CurrentCity], r);
            
        }
        if (mcRaceConfig::g_CurrentCity == CITY::paris)
        {
            max = 6;
            int r = math::RandomInt(min, max);
            sprintf_s(imgName, "%s_%d", g_CityNamesDev[mcRaceConfig::g_CurrentCity], r);
        }
        if (mcRaceConfig::g_CurrentCity == CITY::tokyo)
        {
            max = 7;
            int r = math::RandomInt(min, max);
            sprintf_s(imgName, "%s_%d", g_CityNamesDev[mcRaceConfig::g_CurrentCity], r);
        }

        activity.GetAssets().SetLargeImage(imgName);
        activity.GetAssets().SetLargeText("Midnight Club 2");
        activity.GetAssets().SetSmallImage("mc2_logo");
    }
    break;
    
    case GameState::RaceEditor:
        activity.SetDetails("In race editor");
        activity.SetState(g_CityNames[mcRaceConfig::g_CurrentCity]);
        activity.GetAssets().SetLargeImage("raceeditor");
        activity.GetAssets().SetLargeText("Midnight Club 2");
        activity.GetAssets().SetSmallImage("mc2_logo");
        break;

    default:
        activity.SetDetails("Playing");
        activity.SetState("Midnight Club 2");
        break;
    }

    g_core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {});
}
