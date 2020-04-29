#include "PreferenceSystem.h"

#include "ipreferencesystem.h"
#include "itextstream.h"

#include "modulesystem/StaticModule.h"
#include "ui/prefdialog/PrefDialog.h"

namespace settings
{

IPreferencePage& PreferenceSystem::getPage(const std::string& path)
{
	ensureRootPage();

	return _rootPage->createOrFindPage(path);
}

void PreferenceSystem::foreachPage(const std::function<void(PreferencePage&)>& functor)
{
	ensureRootPage();

	_rootPage->foreachChildPage(functor);
}

void PreferenceSystem::ensureRootPage()
{
	if (!_rootPage)
	{
		_rootPage = std::make_shared<PreferencePage>("");
	}
}

// RegisterableModule implementation
const std::string& PreferenceSystem::getName() const
{
	static std::string _name(MODULE_PREFERENCESYSTEM);
	return _name;
}

const StringSet& PreferenceSystem::getDependencies() const
{
	static StringSet _dependencies;
	return _dependencies;
}

void PreferenceSystem::initialiseModule(const ApplicationContext& ctx)
{
	rMessage() << "PreferenceSystem::initialiseModule called" << std::endl;
}

// Define the static PreferenceSystem module
module::StaticModule<PreferenceSystem> preferenceSystemModule;

}

settings::PreferenceSystem& GetPreferenceSystem()
{
	return *std::static_pointer_cast<settings::PreferenceSystem>(
		module::GlobalModuleRegistry().getModule(MODULE_PREFERENCESYSTEM));
}
