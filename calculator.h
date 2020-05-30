#pragma once

#include "calcbutton.h"
#include "calclabel.h"
#include <QWidget>
#include <QRegularExpression>
#include <QStringList>
#include <QList>
#include <QPair>
#include <QMap>


class Calculator : public QWidget {
	Q_OBJECT
	
	// constants
	const int MAX_PRECISION = 10;
	const int EXP_PRECISION = 3;
	
	// valid event chars for determining type in undo
	const QRegularExpression VALID_DIGIT;
	const QRegularExpression VALID_SCIEN;
	const QRegularExpression VALID_SIGN;
	const QRegularExpression VALID_BINARY;
	const QRegularExpression VALID_UNARY;
	const QRegularExpression VALID_MEM;
	// because matching to QChars is hard for some reason
	const QString VALID_DIGIT_STR = "0123456789.";
	const QString VALID_SCIEN_STR = "e";
	const QString VALID_SIGN_STR = "s";
	const QString VALID_BINARY_STR = "-+xd^lm";
	const QString VALID_UNARY_STR = "ri!";
	const QString VALID_MEM_STR = "MW";
	
	
	// maps binary event chars to their visual string representation
	// required for calling on_binary with undo
	QMap<char, QString> binary_strings;
	
public:
	// constructor
	Calculator(QWidget *parent = 0);
	// destructor
	~Calculator();
	
private slots:
	//--------------------------button press slots---------------------------
	// adds a digit to the active display, also handles decimal points
	// will replace the current value if overwrite is set
	void on_digit(char digit = '\0');
	// adds the scientific notation character 'e+' to the active display
	// can append 'e+' to an overwrite value if it didn't have it before'
	void on_scientific(char scien = '\0');
	// swaps the sign of the number or if hte number has e, the sign of e
	// will still swap if overwrite is set
	void on_sign(char sign = '\0');
	// either writes memory to the display or reads the display value into memory
	// writing to the display triggers overwrite
	void on_memory(char mem = '\0');
	// changes the current binary op to the pressed one, does no calculations
	// sets the active display to lower display, initializes it if not set
	// initializing the lower display triggers overwrite
	void on_binary(char binary_op = '\0');
	// calculates the value of the unary op applied to the display value
	// replaces the display value with the calculated value
	// triggers overwrite, can trigger active_has_error
	void on_unary(char unary_op = '\0');
	// either recalculates the upper display, or does the binary calculation
	// clears the display and places the value in the upper display
	// triggers overwrite, can trigger active_has_error
	void on_equals();
	// clears the display and sets upper_display to "0", triggers overwrite
	void on_clear();
	// returns the calculator to the previous state before the most recent event
	void on_undo();
	
private:
	// display variables that both store and show information to the user
	CalcLabel *upper_display;
	CalcLabel *lower_display;
	CalcLabel *binary_display;
	CalcLabel *memory_display;
	
	// backend variables that store some state information
	// points to upper or lower, whichever is active
	CalcLabel *active_display;
	// the stored memory values
	QString memory1 = "";
	QString memory2 = "";
	// two levels of flagging states: active_has error implies overwrite
	// although overwrite doesn't imply active_has_error
	bool overwrite_on_input = false;
	bool active_has_error = false;
	// the operator to be used by on_equals, char because char is switchable
	char cur_binary_op = '\0';
	
	// variables to implement undo
	// first is the upper display value at screen clear
	// second is the events
	QList<QPair<QString, QString>> event_list;
	// I refuse to recalculate old events when undoing
	// these variables compensate for that limitation
	QStringList old_unary_values;
	QList<QString *> all_mem_values;
	QStringList old_mem1_values;
	QStringList old_mem2_values;
	
	//--------------------------display functions----------------------------
	// clears displays and sets active display to upper
	// also clears error and overwrite flags
	void reset_displays(QString new_upper_str = "0", bool to_add_event = true);
	// updates the memory display based on stored memory strings
	void update_memory_display();
	
	//---------------------------number functions----------------------------
	// self explanatory, see also, encapsulation
	QString double_to_string(const double val);
	// ^
	double string_to_double(const QString str);
	// returns the significant figures of a given string
	// used to check for strings getting too precise
	int num_sig_figs(QString str);
	// gives the length of the string after the exponent
	int exponent_length(QString str);
	// returns the result of cur_binary_op applied to up and lo
	double calculate_binary(const double up, const double lo);
	// returns the result of unary_op applied to value
	double calculate_unary(const char unary_op, double value);
	
	//----------------------------error checkers-----------------------------
	// checks for errors regarding invalid inputs to the binary operator
	// clears the display, shows the error, and returns true if an error is found
	bool check_binary_error(const double up, const double lo);
	// checks for various errors regarding invalid inputs to the unary operator
	// replaces the active display with and returns true if an error is found
	bool check_unary_error(const char unary_op, const double value);
	// checks for value equaling inf, -inf, or nan
	// sets error flags and returns a message if equal, returns value otherwise
	QString check_number_error(QString value);
	
	//------------------------------debuggers--------------------------------
	// prints recent events, current displays, flags, and mem values
	// called in reset_displays
	void print_state();
	// prints all past events and old mem and unary values
	// called in the destructor
	void print_all_events();
	
	//----------------------------undo functions-----------------------------
	// requires that event_char_from_button has been called first
	// appends the given char to event_list.last().last()
	// also updates old mem values and old unary values
	void add_event(const char event);
	// this is read by add_event and is solely set by event_char_from_button
	bool to_add_event;
	// returns input_char if it is not '\0', if so, casts button to 
	// Calcbutton and returns the event_char
	char event_char_from_button(QObject *button, char input_char);
	// assumes nothing about the state of the calculator, reads from event_list,
	// and updates the displays, flags, and memory to reflect the current state
	void reset_state();
	// assumes active_display has been set to the default value
	// takes display_events, finds the most recent overwrite event,
	// updates the display to that point, then redoes all events after that point
	void set_display(const QString &display_events, int unary_offset,
					 int mem_offset);
	// deduces the type of event and calls the corresponding calculator function
	void redo_event(char event);
	
};
