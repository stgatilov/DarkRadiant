#ifndef MRUMENUITEM_H_
#define MRUMENUITEM_H_

#include <string>
#include "icommandsystem.h"

// Forward declaration
namespace Gtk
{
	class Label;
	class Widget;
}

namespace ui {

// Forward declaration
class MRU;

/* greebo: An MRUMenuItem holds the information of a single menu entry,
 * this consists mainly of the GtkWidget* (in the menu).
 * 
 * Use the GtkWidget* operator to retrieve the actual widget. 
 */
class MRUMenuItem 
{
private:
	// The label of this widget
	std::string _label;
	
	// The reference to the main class for loading maps and stuff
	MRU& _mru;
	
	// The number of this MRU item to be displayed
	unsigned int _index;
	
	// The internally stored name and reference to the GtkWidget
	Gtk::Widget* _widget;
	
public:
	// Constructor
	MRUMenuItem(const std::string& label, ui::MRU& _mru, unsigned int index);
	
	// Copy Constructor
	MRUMenuItem(const ui::MRUMenuItem& other);
	
	void setWidget(Gtk::Widget* widget);
	Gtk::Widget* getWidget();
	
	// Triggers loading the map represented by this widget 
	void activate(const cmd::ArgumentList& args);
	
	// Shows/hides the widget
	void show();
	void hide();
	
	// Sets/Retrieves the label
	void setLabel(const std::string& label);
	std::string getLabel() const;
	
	int getIndex() const;

private:
	Gtk::Label* findLabel(Gtk::Widget* widget);
	
}; // class MRUMenuItem
	
} // namespace ui

#endif /*MRUMENUITEM_H_*/
