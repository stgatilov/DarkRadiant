#include "PatchThickenDialog.h"

#include <gtk/gtk.h>
#include "mainframe.h"
#include "string/string.h"

namespace {
	const char* WINDOW_TITLE = "Patch Thicken";
	const char* LABEL_TITLE = "Thicken selected Patches";
	const char* LABEL_THICKNESS_ENTRY = "Thickness (units):";
	const char* LABEL_CREATE_SEAMS = "Create _Seams (\"side walls\")";
	
	const float DEFAULT_THICKNESS = 16.0f;
	const bool DEFAULT_CREATE_SEAMS = true;
}

namespace ui {

PatchThickenDialog::PatchThickenDialog() :
	_parent(MainFrame_getWindow()),
	_dialog(NULL)
{
	// Create the new dialog window with OK and CANCEL button    
  	_dialog = gtk_dialog_new_with_buttons(WINDOW_TITLE, _parent,
                                         GTK_DIALOG_DESTROY_WITH_PARENT, 
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
                                         GTK_STOCK_OK, GTK_RESPONSE_OK,
                                         NULL);
    
    // Set the dialog properties
    gtk_box_set_spacing(GTK_BOX(GTK_DIALOG(_dialog)->vbox), 6);
    gtk_container_set_border_width(GTK_CONTAINER(_dialog), 6);
    gtk_dialog_set_has_separator(GTK_DIALOG(_dialog), false);
    
    // Create the title label (bold font)
    GtkWidget* topLabel = gtk_label_new(NULL);
    std::string markup = std::string("<span weight=\"bold\">") + LABEL_TITLE + "</span>";
    gtk_label_set_markup(GTK_LABEL(topLabel), markup.c_str());
    gtk_misc_set_alignment(GTK_MISC(topLabel), 0.0f, 0.5f);
    gtk_misc_set_padding(GTK_MISC(topLabel), 6, 2);
    
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(_dialog)->vbox), topLabel, true, true, 0);
    
    // Create the entry field
	GtkWidget* thicknessLabel = gtk_label_new(LABEL_THICKNESS_ENTRY);
	gtk_misc_set_alignment(GTK_MISC(thicknessLabel), 0.0f, 0.5f);
	
	_thicknessEntry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(_thicknessEntry), floatToStr(DEFAULT_THICKNESS).c_str());
	
	// Create a new 2x2 table and pack it into an alignment
	GtkWidget* alignment = gtk_alignment_new(0.0f, 0.0f, 1.0f, 1.0f);
	
	// Setup the table with default spacings
	GtkTable* table = GTK_TABLE(gtk_table_new(2, 2, false));
    gtk_table_set_col_spacings(table, 12);
    gtk_table_set_row_spacings(table, 6);
    
    // Indent the table by adding a left-padding to the alignment
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 0, 0, 18, 6); 
    gtk_container_add(GTK_CONTAINER(alignment), GTK_WIDGET(table));
    
    // Pack the thickness entry field into the table
    gtk_table_attach_defaults(table, thicknessLabel, 0, 1, 0, 1);
    gtk_table_attach_defaults(table, _thicknessEntry, 1, 2, 0, 1);
	
	// Create the "create seams" label
	_seamsCheckBox = gtk_check_button_new_with_mnemonic(LABEL_CREATE_SEAMS);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_seamsCheckBox), DEFAULT_CREATE_SEAMS);
	gtk_table_attach_defaults(table, _seamsCheckBox, 0, 2, 1, 2);
	
	// Pack the table into the dialog
	gtk_box_pack_end(GTK_BOX(GTK_DIALOG(_dialog)->vbox), GTK_WIDGET(alignment), true, true, 0);
}

bool PatchThickenDialog::queryPatchThickness(float& thickness, bool& createSeams) {
	gtk_widget_show_all(_dialog);
	gint response = gtk_dialog_run(GTK_DIALOG(_dialog));
	
	bool returnValue = false;
	
	if (response == GTK_RESPONSE_OK) {
		thickness = strToFloat(gtk_entry_get_text(GTK_ENTRY(_thicknessEntry)));
		createSeams = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(_seamsCheckBox));
		returnValue = true;
	}
	
	gtk_widget_destroy(GTK_WIDGET(_dialog));
	
	return returnValue;
}

} // namespace ui
