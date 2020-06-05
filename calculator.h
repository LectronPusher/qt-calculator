#pragma once

#include "calclabel.h"
#include <QWidget>
#include <QRegularExpression>
#include <QStringList>
#include <QKeyEvent>
#include <QList>
#include <QPair>

class Calculator : public QWidget {
	Q_OBJECT
	
public:
	// constructor
	// initializes variables and displays, adds buttons, sets the layout
	Calculator(QWidget *parent = 0);
	// destructor
	// calls print_all_events()
	~Calculator();
	
	// public getters for viewing state
	QString get_upper_text();
	QString get_lower_text();
	QString get_memory1();
	QString get_memory2();
	char get_binary_op();
	
	// calls an input function based on event, also sets add_next_event
	// returns whether the event was recognized by the switch statement
	bool do_event(const char event, bool add_this_event);
	
protected:
	// handles key press events, passes on to QWidget if not recognized
	void keyPressEvent(QKeyEvent *event);
	
private:
	//-------------------------------variables-------------------------------
	// display variables that both store and show information to the user
	CalcLabel *upper_display;
	CalcLabel *lower_display;
	CalcLabel *binary_display;
	CalcLabel *memory_display;
	// points to upper or lower, whichever is active
	CalcLabel *active_display;
	// the stored memory values
	QString memory1;
	QString memory2;
	// error flags: active_has error implies overwrite
	// however overwrite doesn't imply active_has_error
	bool overwrite_on_input = true;
	bool active_has_error = false;
	// the operator to be used by on_equals
	char cur_binary_op = '\0';
	
	// precision constants
	const int MAX_PRECISION = 10;
	const int EXP_PRECISION = 3;
	
	//----------------------------undo variables-----------------------------
	// a frame is whenever the displays are cleared and a value is put in upper
	// first is the value of upper at that frame
	// second is the event list for that frame
	QList<QPair<QString, QString>> event_frames;
	// I refuse to recalculate old events when undoing
	// these variables compensate for that limitation
	QStringList old_unary_values;
	QList<QString *> all_mem_values;
	QStringList old_mem1_values;
	QStringList old_mem2_values;
	// determine binary, unary, and mem locations in undo
	const QRegularExpression VALID_BINARY;
	const QRegularExpression VALID_UNARY;
	const QRegularExpression VALID_MEM;
	
	//----------------------------input functions----------------------------
	// adds a digit to the active display, also handles decimal points
	// will replace the current display if overwrite is set
	void on_digit(const char digit);
	// changes the current binary op to the pressed one, does no calculations
	// sets the active display to lower display, initializes it if not set
	// initializing the lower display triggers overwrite
	void on_binary(const char binary_op);
	// calculates the value of the unary op applied to the display value
	// replaces the display value with the calculated value
	// triggers overwrite, can trigger active_has_error
	void on_unary(const char unary_op);
	// either writes memory to the display or reads the display value into memory
	// writing to the display triggers overwrite
	void on_memory(const char mem);
	// adds the scientific notation character 'e+' to the active display
	// can append 'e+' to an overwrite value if it didn't have it before
	void on_scientific();
	// swaps the sign of the number or if the number has e, the sign of e
	// will still swap if overwrite is set
	void on_sign();
	// either recalculates the upper display, or does the binary calculation
	// clears the display and places the value in the upper display
	// triggers overwrite, can trigger active_has_error
	void on_equals();
	// clears the display and sets upper_display to "0", triggers overwrite
	void on_clear();
	// returns the calculator to the previous state before the most recent event
	void on_undo();
	
	//--------------------------display functions----------------------------
	// clears displays and sets active display to upper
	// triggers overwrite flag, but does not alter active_has_error
	void reset_displays(const QString &reset_val = "0");
	// updates the memory display based on stored memory strings
	void update_memory_display();
	
	//---------------------------number functions----------------------------
	// these handle string conversion
	QString double_to_string(const double val);
	double string_to_double(const QString &str);
	// checks if str is at the max precision
	bool at_max_precision(QString str);
	// returns the result of cur_binary_op applied to up and lo
	double calculate_binary(const double up, const double lo);
	// returns the result of unary_op applied to value
	double calculate_unary(const char unary_op, const double value);
	
	//----------------------------error checkers-----------------------------
	// these all throw a QString containing the error message if an error is found
	// all error messages contain the string "error" in them
	// these functions are solely called by on_equals() and on_unary()
	// which wrap them in try catch blocks to handle the error messages
	// checks for errors regarding invalid inputs to the binary operator
	void check_binary_error(const double up, const double lo);
	// checks for errors regarding invalid inputs to the unary operator
	void check_unary_error(const char unary_op, const double value);
	// checks for value equaling inf, -inf, or nan
	void check_number_error(const QString &value);
	
	//----------------------------undo functions-----------------------------
	// appends the given event to the event list for the current frame
	// also updates old mem values and old unary values
	void add_event(const char event);
	// sets the state of the calculator to that of the last event frame
	// updates the displays, flags, and memory to reflect the new state
	void reset_state();
	// requires active_display has been set to the default value
	// takes display_events, finds the most recent event that triggered overwrite,
	// updates the display to that point, then redoes all events after that point
	void reset_active_display(QString &display_events, const int unary_offset,
							  const int mem_offset);
	
	//------------------------------debuggers--------------------------------
	// prints recent events, current displays, flags, and mem values
	// called in on_clear() and on_equals()
	void print_state();
	// prints past event frames and the values of old mem and old unary
	// called in the destructor
	void print_all_events();
	
};
