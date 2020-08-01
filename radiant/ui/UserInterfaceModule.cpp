#include "UserInterfaceModule.h"

#include "i18n.h"
#include "ilayer.h"
#include "ifilter.h"
#include "ientity.h"
#include "imru.h"
#include "ibrush.h"
#include "ipatch.h"
#include "iorthocontextmenu.h"
#include "ieventmanager.h"
#include "imousetool.h"
#include "imainframe.h"
#include "ishaders.h"

#include "wxutil/menu/CommandMenuItem.h"
#include "wxutil/MultiMonitor.h"
#include "wxutil/dialog/MessageBox.h"
#include "messages/TextureChanged.h"
#include "string/string.h"

#include "module/StaticModule.h"

#include "MapCommands.h"
#include "ui/aas/AasControlDialog.h"
#include "ui/prefdialog/GameSetupDialog.h"
#include "ui/layers/LayerOrthoContextMenuItem.h"
#include "ui/layers/LayerControlDialog.h"
#include "ui/overlay/OverlayDialog.h"
#include "ui/prefdialog/PrefDialog.h"
#include "ui/Documentation.h"
#include "log/Console.h"
#include "ui/lightinspector/LightInspector.h"
#include "ui/patch/PatchInspector.h"
#include "ui/surfaceinspector/SurfaceInspector.h"
#include "ui/transform/TransformDialog.h"
#include "ui/findshader/FindShader.h"
#include "ui/mapinfo/MapInfoDialog.h"
#include "ui/commandlist/CommandList.h"
#include "ui/mousetool/ToolMappingDialog.h"
#include "ui/about/AboutDialog.h"
#include "ui/eclasstree/EClassTree.h"
#include "ui/entitylist/EntityList.h"
#include "ui/particles/ParticleEditor.h"
#include "ui/patch/CapDialog.h"
#include "ui/patch/PatchThickenDialog.h"
#include "textool/TexTool.h"
#include "modelexport/ExportAsModelDialog.h"
#include "modelexport/ExportCollisionModelDialog.h"
#include "ui/filters/FilterOrthoContextMenuItem.h"
#include "uimanager/colourscheme/ColourSchemeEditor.h"
#include "ui/layers/CreateLayerDialog.h"
#include "ui/patch/PatchCreateDialog.h"
#include "ui/patch/BulgePatchDialog.h"
#include "ui/selectionset/SelectionSetToolmenu.h"
#include "ui/brush/QuerySidesDialog.h"
#include "ui/brush/FindBrush.h"
#include "ui/mousetool/RegistrationHelper.h"

#include <wx/version.h>

namespace ui
{

namespace
{
	const char* const LAYER_ICON = "layers.png";
	const char* const CREATE_LAYER_TEXT = N_("Create Layer...");

	const char* const ADD_TO_LAYER_TEXT = N_("Add to Layer...");
	const char* const MOVE_TO_LAYER_TEXT = N_("Move to Layer...");
	const char* const REMOVE_FROM_LAYER_TEXT = N_("Remove from Layer...");

	const char* const SELECT_BY_FILTER_TEXT = N_("Select by Filter...");
	const char* const DESELECT_BY_FILTER_TEXT = N_("Deselect by Filter...");
}

const std::string& UserInterfaceModule::getName() const
{
	static std::string _name("UserInterfaceModule");
	return _name;
}

const StringSet& UserInterfaceModule::getDependencies() const
{
	static StringSet _dependencies;

	if (_dependencies.empty())
	{
		_dependencies.insert(MODULE_LAYERS);
		_dependencies.insert(MODULE_ORTHOCONTEXTMENU);
		_dependencies.insert(MODULE_UIMANAGER);
		_dependencies.insert(MODULE_FILTERSYSTEM);
		_dependencies.insert(MODULE_ENTITY);
		_dependencies.insert(MODULE_EVENTMANAGER);
		_dependencies.insert(MODULE_RADIANT_CORE);
		_dependencies.insert(MODULE_MRU_MANAGER);
		_dependencies.insert(MODULE_MOUSETOOLMANAGER);
	}

	return _dependencies;
}

void UserInterfaceModule::initialiseModule(const ApplicationContext& ctx)
{
	rMessage() << getName() << "::initialiseModule called." << std::endl;

	// Output the wxWidgets version to the logfile
	std::string wxVersion = string::to_string(wxMAJOR_VERSION) + ".";
	wxVersion += string::to_string(wxMINOR_VERSION) + ".";
	wxVersion += string::to_string(wxRELEASE_NUMBER);

	rMessage() << "wxWidgets Version: " << wxVersion << std::endl;

	wxutil::MultiMonitor::printMonitorInfo();

	registerUICommands();

	// Register LayerControlDialog
	GlobalCommandSystem().addCommand("ToggleLayerControlDialog", LayerControlDialog::toggle);

	// Create a new menu item connected to the CreateNewLayer command
	GlobalOrthoContextMenu().addItem(std::make_shared<wxutil::CommandMenuItem>(
			new wxutil::IconTextMenuItem(_(CREATE_LAYER_TEXT), LAYER_ICON), "CreateNewLayer"),
		IOrthoContextMenu::SECTION_LAYER
	);

	// Add the orthocontext menu's layer actions
	GlobalOrthoContextMenu().addItem(
		std::make_shared<LayerOrthoContextMenuItem>(_(ADD_TO_LAYER_TEXT), 
			LayerOrthoContextMenuItem::AddToLayer),
		IOrthoContextMenu::SECTION_LAYER
	);

	GlobalOrthoContextMenu().addItem(
		std::make_shared<LayerOrthoContextMenuItem>(_(MOVE_TO_LAYER_TEXT), 
			LayerOrthoContextMenuItem::MoveToLayer),
		IOrthoContextMenu::SECTION_LAYER
	);

	GlobalOrthoContextMenu().addItem(
		std::make_shared<LayerOrthoContextMenuItem>(_(REMOVE_FROM_LAYER_TEXT), 
			LayerOrthoContextMenuItem::RemoveFromLayer),
		IOrthoContextMenu::SECTION_LAYER
	);

	GlobalRadiant().signal_radiantStarted().connect(
		sigc::ptr_fun(LayerControlDialog::onRadiantStartup));

	// Add the filter actions
	GlobalOrthoContextMenu().addItem(
		std::make_shared<FilterOrthoContextMenuItem>(_(SELECT_BY_FILTER_TEXT),
			FilterOrthoContextMenuItem::SelectByFilter),
		IOrthoContextMenu::SECTION_FILTER
	);

	GlobalOrthoContextMenu().addItem(
		std::make_shared<FilterOrthoContextMenuItem>(_(DESELECT_BY_FILTER_TEXT),
			FilterOrthoContextMenuItem::DeselectByFilter),
		IOrthoContextMenu::SECTION_FILTER
	);

	_eClassColourManager.reset(new EntityClassColourManager);
	_longOperationHandler.reset(new LongRunningOperationHandler);
	_mapFileProgressHandler.reset(new MapFileProgressHandler);
	_autoSaveRequestHandler.reset(new AutoSaveRequestHandler);
	_fileSelectionRequestHandler.reset(new FileSelectionRequestHandler);

	initialiseEntitySettings();

	_execFailedListener = GlobalRadiantCore().getMessageBus().addListener(
		radiant::IMessage::Type::CommandExecutionFailed,
		radiant::TypeListener<radiant::CommandExecutionFailedMessage>(
			sigc::mem_fun(this, &UserInterfaceModule::handleCommandExecutionFailure)));

	_textureChangedListener = GlobalRadiantCore().getMessageBus().addListener(
		radiant::IMessage::Type::TextureChanged,
		radiant::TypeListener(UserInterfaceModule::HandleTextureChanged));

	_notificationListener = GlobalRadiantCore().getMessageBus().addListener(
		radiant::IMessage::Type::Notification,
		radiant::TypeListener(UserInterfaceModule::HandleNotificationMessage));

	// Initialise the AAS UI
	AasControlDialog::Init();

	SelectionSetToolmenu::Init();

	_mruMenu.reset(new MRUMenu);
	_shaderClipboardStatus.reset(new ShaderClipboardStatus);

	MouseToolRegistrationHelper::RegisterTools();
}

void UserInterfaceModule::shutdownModule()
{
	GlobalRadiantCore().getMessageBus().removeListener(_textureChangedListener);
	GlobalRadiantCore().getMessageBus().removeListener(_execFailedListener);
	GlobalRadiantCore().getMessageBus().removeListener(_notificationListener);

	_coloursUpdatedConn.disconnect();
	_entitySettingsConn.disconnect();

	_longOperationHandler.reset();
	_eClassColourManager.reset();
	_mapFileProgressHandler.reset();
	_fileSelectionRequestHandler.reset();
	_autoSaveRequestHandler.reset();
	_shaderClipboardStatus.reset();
	
	_mruMenu.reset();
}

void UserInterfaceModule::handleCommandExecutionFailure(radiant::CommandExecutionFailedMessage& msg)
{
	auto parentWindow = module::GlobalModuleRegistry().moduleExists(MODULE_MAINFRAME) ?
		GlobalMainFrame().getWxTopLevelWindow() : nullptr;

	wxutil::Messagebox::ShowError(msg.getMessage(), parentWindow);
}

void UserInterfaceModule::HandleNotificationMessage(radiant::NotificationMessage& msg)
{
	auto parentWindow = module::GlobalModuleRegistry().moduleExists(MODULE_MAINFRAME) ?
		GlobalMainFrame().getWxTopLevelWindow() : nullptr;

	switch (msg.getType())
	{
	case radiant::NotificationMessage::Information:
		wxutil::Messagebox::Show(msg.hasTitle() ? msg.getTitle() : _("Notification"), 
			msg.getMessage(), IDialog::MessageType::MESSAGE_CONFIRM, parentWindow);
		break;

	case radiant::NotificationMessage::Warning:
		wxutil::Messagebox::Show(msg.hasTitle() ? msg.getTitle() : _("Warning"), 
			msg.getMessage(), IDialog::MessageType::MESSAGE_WARNING, parentWindow);
		break;

	case radiant::NotificationMessage::Error:
		wxutil::Messagebox::Show(msg.hasTitle() ? msg.getTitle() : _("Error"), 
			msg.getMessage(), IDialog::MessageType::MESSAGE_ERROR, parentWindow);
		break;
	};
}

void UserInterfaceModule::initialiseEntitySettings()
{
	auto& settings = GlobalEntityModule().getSettings();

	_entitySettingsConn = settings.signal_settingsChanged().connect(
		[]() { GlobalMainFrame().updateAllWindows(); }
	);

	applyEntityVertexColours();
	applyBrushVertexColours();
	applyPatchVertexColours();

	_coloursUpdatedConn = ColourSchemeEditor::signal_ColoursChanged().connect(
		[this]() { 
			applyEntityVertexColours(); 
			applyBrushVertexColours(); 
			applyPatchVertexColours();
		}
	);

	GlobalEventManager().addRegistryToggle("ToggleShowAllLightRadii", RKEY_SHOW_ALL_LIGHT_RADII);
	GlobalEventManager().addRegistryToggle("ToggleShowAllSpeakerRadii", RKEY_SHOW_ALL_SPEAKER_RADII);
	GlobalEventManager().addRegistryToggle("ToggleDragResizeEntitiesSymmetrically", RKEY_DRAG_RESIZE_SYMMETRICALLY);
}

void UserInterfaceModule::applyBrushVertexColours()
{
	auto& settings = GlobalBrushCreator().getSettings();

	settings.setVertexColour(ColourSchemes().getColour("brush_vertices"));
}

void UserInterfaceModule::applyPatchVertexColours()
{
	auto& settings = GlobalPatchModule().getSettings();

	settings.setVertexColour(patch::PatchEditVertexType::Corners, ColourSchemes().getColour("patch_vertex_corner"));
	settings.setVertexColour(patch::PatchEditVertexType::Inside, ColourSchemes().getColour("patch_vertex_inside"));
}

void UserInterfaceModule::applyEntityVertexColours()
{
	auto& settings = GlobalEntityModule().getSettings();

	settings.setLightVertexColour(LightEditVertexType::StartEndDeselected, ColourSchemes().getColour("light_startend_deselected"));
	settings.setLightVertexColour(LightEditVertexType::StartEndSelected, ColourSchemes().getColour("light_startend_selected"));
	settings.setLightVertexColour(LightEditVertexType::Inactive, ColourSchemes().getColour("light_vertex_normal"));
	settings.setLightVertexColour(LightEditVertexType::Deselected, ColourSchemes().getColour("light_vertex_deselected"));
	settings.setLightVertexColour(LightEditVertexType::Selected, ColourSchemes().getColour("light_vertex_selected"));
}

void UserInterfaceModule::refreshShadersCmd(const cmd::ArgumentList& args)
{
	// Disable screen updates for the scope of this function
	auto blocker = GlobalMainFrame().getScopedScreenUpdateBlocker(_("Processing..."), _("Loading Shaders"));

	// Reload the Shadersystem, this will also trigger an 
	// OpenGLRenderSystem unrealise/realise sequence as the rendersystem
	// is attached to this class as Observer
	// We can't do this refresh() operation in a thread it seems due to context binding
	GlobalMaterialManager().refresh();

	GlobalMainFrame().updateAllWindows();
}

void UserInterfaceModule::registerUICommands()
{
	TexTool::registerCommands();

	GlobalCommandSystem().addCommand("ProjectSettings", GameSetupDialog::Show);
	GlobalCommandSystem().addCommand("Preferences", PrefDialog::ShowPrefDialog);

	GlobalCommandSystem().addCommand("ToggleConsole", Console::toggle);
	GlobalCommandSystem().addCommand("ToggleLightInspector", LightInspector::toggleInspector);
	GlobalCommandSystem().addCommand("SurfaceInspector", SurfaceInspector::toggle);
	GlobalCommandSystem().addCommand("PatchInspector", PatchInspector::toggle);
	GlobalCommandSystem().addCommand("OverlayDialog", OverlayDialog::toggle);
	GlobalCommandSystem().addCommand("TransformDialog", TransformDialog::toggle);

	GlobalCommandSystem().addCommand("FindBrush", FindBrushDialog::Show);

	GlobalCommandSystem().addCommand("MapInfo", MapInfoDialog::ShowDialog);
	GlobalCommandSystem().addCommand("MouseToolMappingDialog", ToolMappingDialog::ShowDialog);

	GlobalCommandSystem().addCommand("FindReplaceTextures", FindAndReplaceShader::ShowDialog);
	GlobalCommandSystem().addCommand("ShowCommandList", CommandList::ShowDialog);
	GlobalCommandSystem().addCommand("About", AboutDialog::showDialog);
	GlobalCommandSystem().addCommand("ShowUserGuide", Documentation::showUserGuide);
	GlobalCommandSystem().addCommand("ExportSelectedAsModelDialog", ExportAsModelDialog::ShowDialog);

	GlobalCommandSystem().addCommand("EntityClassTree", EClassTree::ShowDialog);
	GlobalCommandSystem().addCommand("EntityList", EntityList::toggle);

	GlobalCommandSystem().addCommand("RefreshShaders",
		std::bind(&UserInterfaceModule::refreshShadersCmd, this, std::placeholders::_1));

	// Add the callback event
	GlobalCommandSystem().addCommand("ParticlesEditor", ParticleEditor::DisplayDialog);

	// Register the "create layer" command
	GlobalCommandSystem().addCommand("CreateNewLayer", CreateLayerDialog::CreateNewLayer,
		{ cmd::ARGTYPE_STRING | cmd::ARGTYPE_OPTIONAL });

	GlobalCommandSystem().addCommand("BulgePatchDialog", BulgePatchDialog::BulgePatchCmd);
	GlobalCommandSystem().addCommand("PatchCapDialog", PatchCapDialog::Show);
	GlobalCommandSystem().addCommand("ThickenPatchDialog", PatchThickenDialog::Show);
	GlobalCommandSystem().addCommand("CreateSimplePatchDialog", PatchCreateDialog::Show);

	GlobalCommandSystem().addCommand("ExportCollisionModelDialog", ExportCollisionModelDialog::Show);
	GlobalCommandSystem().addCommand("QueryBrushPrefabSidesDialog", QuerySidesDialog::Show, { cmd::ARGTYPE_INT });

	// Set up the CloneSelection command to react on key up events only
	GlobalEventManager().addCommand("CloneSelection", "CloneSelection", true); // react on keyUp

	GlobalEventManager().addRegistryToggle("ToggleRotationPivot", "user/ui/rotationPivotIsOrigin");
	GlobalEventManager().addRegistryToggle("ToggleSnapRotationPivot", "user/ui/snapRotationPivotToGrid");
	GlobalEventManager().addRegistryToggle("ToggleOffsetClones", "user/ui/offsetClonedObjects");
	GlobalEventManager().addRegistryToggle("ToggleFreeObjectRotation", RKEY_FREE_OBJECT_ROTATION);

	GlobalCommandSystem().addCommand("LoadPrefab", ui::loadPrefabDialog);
}

void UserInterfaceModule::HandleTextureChanged(radiant::TextureChangedMessage& msg)
{
	SurfaceInspector::update();
}

// Static module registration
module::StaticModule<UserInterfaceModule> userInterfaceModule;

}
