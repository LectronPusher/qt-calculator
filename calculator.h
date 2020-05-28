#pragma once

#include "calcbutton.h"
#include "calclabel.h"
#include <QWidget>
#include <QList>
#include <QStringList>
#include <QMap>

#include <string>


class Calculator : public QWidget {
	Q_OBJECT
	
	// constants
	const int MAX_PRECISION = 11;
	const int EXP_PRECISION = 3;
	
	// valid event chars for determining type in undo
	const std::string VALID_DIGIT = "0123456789.";
	const std::string VALID_SCIEN = "e";
	const std::string VALID_BINARY = "+-xd^lm";
	const std::string VALID_UNARY = "ri!";
	const std::string VALID_SIGN = "s";
	const std::string VALID_MEM = "MW";
	
	// maps binary event chars to their visual string representation
	// required for calling on_binary with undo
	QMap<char, QString> binary_strings;
	
public:
	// constructor
	Calculator(QWidget *parent = 0);
	
private slots:
	//--------------------------button press slots---------------------------
	// if a button has a default input, if the value is that default, it will
	// assume it was pressed by a CalcButton and change the input to the
	// event_char of the CalcButton
	
	// adds a digit to the active display, also handles decimal points
	void on_digit(char digit = '\0');
	// 'e' is just different enough from other digits to deserve it's own function
	void on_scientific(char scien = '\0');
	// swaps the sign of the number or the sign of e
	void on_sign(char sign = '\0');
	// changes the current binary op to the pressed one, does no calculations
	void on_binary(char binary_op = '\0');
	// either puts memory value in the display or puts display value in memory
	// putting in the display triggers overwrite
	void on_memory(char mem = '\0');
	// calculates the value of the unary op applied to the display value
	// replaces the display value with the calculated one
	// triggers overwrite_on_input, conditionally triggers active_has_error
	void on_unary(char unary_op = '\0');
	// clears all inputs and sets active input to upper, also clears error flags
	void on_clear();
	// either recalculates the upper display, or does the binary calculation
	// clears the display and places the value in the upper display
	// triggers overwrite_on_input, conditionally triggers active_has_error
	void on_equals();
	// undoes the previous action in the event list
	void on_undo();
	
private:
	// display variables that both store and show information to the user
	CalcLabel *upper_display;
	CalcLabel *lower_display;
	CalcLabel *binary_display;
	CalcLabel *memory_display;
	
	// backend variables that store calculator state
	// points to upper or lower, whicever is active
	CalcLabel *active_display;
	char cur_binary_op = '\0';
	// two levels of error states, active_has error implies overwrite
	// although overwrite doesn't imply active_has_error'
	bool overwrite_on_input = false;
	bool active_has_error = false;
	QString memory1 = "";
	QString memory2 = "";
	
	// variables to implement undo
	// using std::string for char compatability because I like switch statements
	// first string is the upper display value at screen clear
	// last string is the events
	QList<QList<std::string>> event_list = { { "", "" } };
	// I refuse to recalculate old events when undoing
	// these variables compensate for that limitation
	QStringList old_mem1_values = { "" };
	QStringList old_mem2_values = { "" };
	QStringList old_unary_values  = { "" };
	
	//----------------------------usage functions----------------------------
	// clears displays and sets active display to upper
	// also clears error and overwrite flags
	void clear_displays(QString new_upper_str = "");
	// updates the memory display based on stored memory strings
	void update_memory_display();
	
	//---------------------------number functions----------------------------
	// converts a double to a QString, uses scientific notation if necessary
	// also handles max string sizes
	QString double_to_string(const double val);
	// does the opposite of ^
	double string_to_double(const QString str);
	// returns the significant figures of the given string
	int num_sig_figs(QString str);
	// gives the length of the string after the exponent
	int exponent_length(QString str);
	// returns the result of cur_binary_op applied to up and lo
	double calculate_binary(const double up, const double lo);
	// returns the result of unary_op applied to value
	double calculate_unary(const char unary_op, double value);
	
	//----------------------------error checkers-----------------------------
	// checks for various errors regarding invalid inputs to the binary operator
	// clears the display, shows the error, and returns true if an error is found
	bool check_binary_error(const double up, const double lo);
	// checks for various errors regarding invalid inputs to the unary operator
	// replaces the active display with and returns true if an error is found
	bool check_unary_error(const char unary_op, const double value);
	// checks for value equaling inf, -inf, or nan, if so replaces it with an 
	// error message, otherwise returns value
	QString check_number_error(QString value);
	
	//----------------------------undo functions-----------------------------
	// requires that event_char_from_button has been called before
	// appends the given char to event_list.last().last()
	// also updates old mem values
	void add_event(const char event);
	// this is read by add_event and is solely set by event_char_from_button
	bool to_add_event;
	// returns input_char if it is not '\0', if so, casts button to 
	// Calcbutton and returns the event_char
	char event_char_from_button(QObject *button, char input_char);
	// prints recent events, current displays, flags, and mem values
	void print_recent_events();
	// does debugging by printing all past events and old mem and unary values
	void print_all_events();
	
};
