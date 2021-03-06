
AM_CPPFLAGS = -DPKGLIBDIR='"$(pkglibdir)"' \
              -DPKGDATADIR='"$(pkgdatadir)"' \
              -DLOCALEDIR='"$(localedir)"' \
              -DHTMLDIR='"$(htmldir)"' \
              -I$(top_srcdir)/include \
              -I$(top_srcdir)/libs \
              $(LIBSIGC_CFLAGS) \
              $(XML_CFLAGS) \
              $(FTGL_CFLAGS)

# Clusters of source files common to executable and tests
#VFS_SOURCES = vfs/DeflatedInputStream.cpp \
#              vfs/DirectoryArchive.cpp \
#              vfs/Doom3FileSystem.cpp \
#              vfs/ZipArchive.cpp
#SHADERS_SOURCES = shaders/Doom3ShaderLayer.cpp \
#                  shaders/TableDefinition.cpp \
#                  shaders/textures/GLTextureManager.cpp

# DarkRadiant executable
bin_PROGRAMS = darkradiant
darkradiant_CPPFLAGS = $(AM_CPPFLAGS) 
darkradiant_LDFLAGS = $(XML_LIBS) \
                      $(GLEW_LIBS) \
                      $(GL_LIBS) \
                      $(GLU_LIBS) \
                      $(JPEG_LIBS) \
                      $(PNG_LIBS) \
                      $(FTGL_LIBS) \
                      $(LIBSIGC_LIBS) \
                      $(FILESYSTEM_LIBS) \
                      $(DL_LIBS) \
                      $(INTL_LIBS) \
                      $(WX_LIBS) \
                      $(Z_LIBS)
darkradiant_LDADD = $(top_builddir)/libs/scene/libscenegraph.la \
                    $(top_builddir)/libs/wxutil/libwxutil.la \
                    $(top_builddir)/libs/xmlutil/libxmlutil.la \
                    $(top_builddir)/libs/math/libmath.la \
                    $(top_builddir)/libs/module/libmodule.la
darkradiant_SOURCES = main.cpp \
                      RadiantApp.cpp \
                      RadiantModule.cpp \
                      camera/Camera.cpp \
                      camera/GlobalCamera.cpp \
                      camera/CameraSettings.cpp \
                      camera/CamRenderer.cpp \
                      camera/CamWnd.cpp \
                      camera/FloatingCamWnd.cpp \
                      eventmanager/Accelerator.cpp \
                      eventmanager/EventManager.cpp \
                      eventmanager/GlobalKeyEventFilter.cpp \
                      eventmanager/MouseToolGroup.cpp \
                      eventmanager/MouseToolManager.cpp \
                      eventmanager/Statement.cpp \
                      eventmanager/Toggle.cpp \
                      eventmanager/WidgetToggle.cpp \
                      render/OpenGLModule.cpp \
                      render/View.cpp \
					  ui/UserInterfaceModule.cpp \
                      ui/Documentation.cpp \
                      ui/entitychooser/EntityClassChooser.cpp \
                      ui/entitychooser/EntityClassTreePopulator.cpp \
                      ui/eclasstree/EClassTree.cpp \
                      ui/eclasstree/EClassTreeBuilder.cpp \
                      ui/entitylist/EntityList.cpp \
                      ui/entitylist/GraphTreeModel.cpp \
                      ui/prefabselector/PrefabPopulator.cpp \
                      ui/prefabselector/PrefabSelector.cpp \
                      ui/texturebrowser/TextureBrowser.cpp \
                      ui/texturebrowser/TextureBrowserManager.cpp \
                      ui/findshader/FindShader.cpp \
                      ui/mapinfo/MapInfoDialog.cpp \
                      ui/mapinfo/EntityInfoTab.cpp \
                      ui/mapinfo/ShaderInfoTab.cpp \
                      ui/mapinfo/ModelInfoTab.cpp \
					  ui/mapinfo/LayerInfoTab.cpp \
                      ui/mediabrowser/MediaBrowser.cpp \
                      ui/particles/ParticleEditor.cpp \
                      ui/particles/ParticlesChooser.cpp \
                      ui/brush/QuerySidesDialog.cpp \
                      ui/brush/FindBrush.cpp \
                      ui/overlay/OverlayDialog.cpp \
                      ui/overlay/Overlay.cpp \
                      ui/splash/Splash.cpp \
					  ui/grid/GridUserInterface.cpp \
                      ui/layers/CreateLayerDialog.cpp \
                      ui/layers/LayerControl.cpp \
                      ui/layers/LayerContextMenu.cpp \
                      ui/layers/LayerControlDialog.cpp \
                      ui/layers/LayerOrthoContextMenuItem.cpp \
                      ui/ortho/OrthoContextMenu.cpp \
                      ui/about/AboutDialog.cpp \
                      ui/einspector/AnglePropertyEditor.cpp \
                      ui/einspector/TexturePropertyEditor.cpp \
                      ui/einspector/EntityInspector.cpp \
                      ui/einspector/SkinChooser.cpp \
                      ui/einspector/ColourPropertyEditor.cpp \
                      ui/einspector/EntityPropertyEditor.cpp \
                      ui/einspector/SkinPropertyEditor.cpp \
                      ui/einspector/Vector3PropertyEditor.cpp \
                      ui/einspector/AddPropertyDialog.cpp \
                      ui/einspector/FloatPropertyEditor.cpp \
                      ui/einspector/BooleanPropertyEditor.cpp \
                      ui/einspector/LightTextureChooser.cpp \
                      ui/einspector/SoundPropertyEditor.cpp \
                      ui/einspector/PropertyEditor.cpp \
                      ui/einspector/PropertyEditorFactory.cpp \
                      ui/einspector/ClassnamePropertyEditor.cpp \
                      ui/einspector/ModelPropertyEditor.cpp \
                      ui/common/TexturePreviewCombo.cpp \
                      ui/common/ShaderSelector.cpp \
                      ui/common/ShaderChooser.cpp \
                      ui/common/ShaderDefinitionView.cpp \
                      ui/common/RenderableAABB.cpp \
                      ui/common/CommandEntry.cpp \
                      ui/common/EntityChooser.cpp \
                      ui/common/MapPreview.cpp \
                      ui/filters/FilterContextMenu.cpp \
                      ui/filters/FilterOrthoContextMenuItem.cpp \
                      ui/filters/FilterUserInterface.cpp \
                      ui/filters/FiltersMainMenu.cpp \
                      ui/filters/editor/FilterDialog.cpp \
                      ui/filters/editor/FilterEditor.cpp \
                      ui/prefdialog/GameSetupDialog.cpp \
					  ui/prefdialog/GameSetupPage.cpp \
                      ui/prefdialog/GameSetupPageIdTech.cpp \
					  ui/prefdialog/GameSetupPageTdm.cpp \
                      ui/prefdialog/PrefPage.cpp \
                      ui/prefdialog/PrefDialog.cpp \
					  ui/prefdialog/PreferenceItem.cpp \
                      ui/transform/TransformDialog.cpp \
                      ui/modelselector/MaterialsList.cpp \
                      ui/modelselector/ModelSelector.cpp \
                      ui/surfaceinspector/SurfaceInspector.cpp \
                      ui/patch/CapDialog.cpp \
                      ui/patch/PatchThickenDialog.cpp \
                      ui/patch/PatchInspector.cpp \
                      ui/patch/PatchCreateDialog.cpp \
                      ui/patch/BulgePatchDialog.cpp \
                      ui/lightinspector/LightInspector.cpp \
                      ui/commandlist/CommandList.cpp \
                      ui/commandlist/ShortcutChooser.cpp \
                      ui/mainframe/MainFrame.cpp \
                      ui/mainframe/MainFrameLayoutManager.cpp \
                      ui/mainframe/FloatingLayout.cpp \
                      ui/mainframe/EmbeddedLayout.cpp \
                      ui/mainframe/SplitPaneLayout.cpp \
                      ui/mainframe/RegularLayout.cpp \
                      ui/mainframe/ScreenUpdateBlocker.cpp \
                      ui/mainframe/TopLevelFrame.cpp \
                      ui/modelexport/ExportAsModelDialog.cpp \
                      ui/modelexport/ExportCollisionModelDialog.cpp \
                      ui/mousetool/BindToolDialog.cpp \
                      ui/mousetool/ToolMappingDialog.cpp \
					  ui/aas/AasControl.cpp \
					  ui/aas/AasControlDialog.cpp \
                      ui/aas/RenderableAasFile.cpp \
                      ui/selectionset/SelectionSetToolmenu.cpp \
                      ui/LongRunningOperationHandler.cpp \
                      ui/MapCommands.cpp \
                      ui/MapFileProgressHandler.cpp \
                      ui/statusbar/EditingStopwatchStatus.cpp \
                      uimanager/SoundChooser.cpp \
                      uimanager/StatusBarManager.cpp \
                      uimanager/animationpreview/MD5AnimationChooser.cpp \
                      uimanager/animationpreview/AnimationPreview.cpp \
                      uimanager/animationpreview/MD5AnimationViewer.cpp \
                      uimanager/colourscheme/ColourSchemeEditor.cpp \
                      uimanager/colourscheme/ColourSchemeManager.cpp \
                      uimanager/colourscheme/ColourScheme.cpp \
                      uimanager/ToolbarManager.cpp \
                      uimanager/DialogManager.cpp \
                      ui/filters/FilterMenu.cpp \
                      uimanager/GroupDialog.cpp \
                      uimanager/menu/MenuManager.cpp \
                      uimanager/menu/MenuFolder.cpp \
                      uimanager/menu/MenuSeparator.cpp \
                      uimanager/menu/MenuItem.cpp \
                      uimanager/menu/MenuBar.cpp \
                      uimanager/menu/MenuElement.cpp \
                      uimanager/UIManager.cpp \
                      uimanager/SoundShaderPreview.cpp \
                      xyview/tools/BrushCreatorTool.cpp \
                      xyview/tools/ClipperTool.cpp \
					  xyview/tools/MeasurementTool.cpp \
                      xyview/XYWnd.cpp \
					  xyview/FloatingOrthoView.cpp \
                      xyview/GlobalXYWnd.cpp \
                      textool/TexToolItem.cpp \
                      textool/item/BrushItem.cpp \
                      textool/item/FaceItem.cpp \
                      textool/item/FaceVertexItem.cpp \
                      textool/item/PatchItem.cpp \
                      textool/item/PatchVertexItem.cpp \
                      textool/TexTool.cpp \
                      modulesystem/ApplicationContextImpl.cpp \
                      selection/ManipulateMouseTool.cpp \
                      selection/SelectionMouseTools.cpp \
                      selection/SelectionTest.cpp \
                      settings/Win32Registry.cpp \
                      settings/LocalisationModule.cpp \
                      settings/LocalisationProvider.cpp \
                      map/StartupMapLoader.cpp \
                      log/Console.cpp

check_PROGRAMS = facePlaneTest vfsTest shadersTest
TESTS = $(check_PROGRAMS)

facePlaneTest_SOURCES = test/facePlaneTest.cpp \
                        brush/FacePlane.cpp
facePlaneTest_LDADD = $(top_builddir)/libs/math/libmath.la

#vfsTest_SOURCES = test/vfsTest.cpp $(VFS_SOURCES)
#vfsTest_LDFLAGS = $(FILESYSTEM_LIBS) $(Z_LIBS)

#shadersTest_SOURCES = test/shadersTest.cpp $(SHADERS_SOURCES) #$(VFS_SOURCES)
#shadersTest_LDFLAGS = $(FILESYSTEM_LIBS) $(Z_LIBS)
