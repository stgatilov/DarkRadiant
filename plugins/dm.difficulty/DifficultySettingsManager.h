#pragma once

#include "DifficultySettings.h"

namespace difficulty {

/**
 * greebo: The DifficultyManager contains all settings of all
 *         available difficulty levels. The manager also provides
 *         methods to fill in the settings data into a given
 *         GtkTreeModel and to save the existing settings to the map/entityDef.
 **/
class DifficultySettingsManager
{
    // This contains all the settings of all the difficulty levels
    std::vector<DifficultySettingsPtr> _settings;

    // The names of the difficulty levels
    std::vector<std::string> _difficultyNames;
    std::vector<std::string> _defaultDifficultyNames;

public:
    // Loads all settings from the entityDefs and the currently loaded map.
    void loadSettings();

    // Saves the current working set into one map entity
    void saveSettings();

    // Get the settings object for the given difficulty <level>
    DifficultySettingsPtr getSettings(int level);

    // Get the display name of the given difficulty level
    std::string getDifficultyName(int level);

    /// Set the name for the given difficulty level, and save it into the map
    void setDifficultyName(int level, const std::string& name);

private:
    void loadDefaultSettings();
    void loadMapSettings();
    void loadDifficultyNames();
};

} // namespace difficulty
