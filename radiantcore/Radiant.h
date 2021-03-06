#pragma once

#include "iradiant.h"
#include "modulesystem/ModuleRegistry.h"

namespace applog { class LogFile; }
namespace language { class LanguageManager; }

namespace radiant
{

class MessageBus;

class Radiant :
	public IRadiant
{
private:
	ApplicationContext& _context;

	std::unique_ptr<applog::LogFile> _logFile;

	std::unique_ptr<module::ModuleRegistry> _moduleRegistry;

	std::unique_ptr<MessageBus> _messageBus;

	std::unique_ptr<language::LanguageManager> _languageManager;

public:
	Radiant(ApplicationContext& context);

	~Radiant();

	const std::string& getName() const override;
	const StringSet& getDependencies() const override;
	void initialiseModule(const ApplicationContext& ctx) override;

	applog::ILogWriter& getLogWriter() override;
	module::ModuleRegistry& getModuleRegistry() override;
	radiant::IMessageBus& getMessageBus() override;
	language::ILanguageManager& getLanguageManager() override;
	void startup() override;

	static std::shared_ptr<Radiant>& InstancePtr();

private:
	void createLogFile();
};

}