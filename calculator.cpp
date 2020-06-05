#include "calculator.h"
#include "calclabel.h"
#include "calcbutton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMap>

#include <cmath>

// for debugging
#include <QTextStream>
static QTextStream out(stdout);

//----------------------------constructor----------------------------

// initializes variables and displays, adds buttons, sets the layout
Calculator::Calculator(QWidget *parent) 
	: QWidget(parent),
	VALID_BINARY("[+\\-xd^lm]"),
	VALID_UNARY("[ri!]"),
	VALID_MEM("[MW]")
{	
	// set keyboard focus
	setFocusPolicy(Qt::StrongFocus);
	
	//----------------------display widgets------------------------
	
	upper_display = new CalcLabel(true, this);
	lower_display = new CalcLabel(true, this);
	binary_display = new CalcLabel(false, this);
	memory_display = new CalcLabel(false, this);
	memory_display->setFrameStyle(QFrame::NoFrame);
	
	// set the displays to their initial states
	on_clear();
	update_memory_display();
	
	QGridLayout *displays = new QGridLayout;
	displays->addWidget(memory_display, 0, 0);
	displays->addWidget(upper_display, 0, 1);
	displays->addWidget(binary_display, 1, 0);
	displays->addWidget(lower_display, 1, 1);
	
	//-----------------------button widgets------------------------
	QGridLayout *buttons = new QGridLayout;
	// row 1
	buttons->addWidget(new CalcButton("M1", 'M', this), 0, 0);
	buttons->addWidget(new CalcButton("M2", 'W', this), 0, 1);
	QHBoxLayout *undo_clear_box = new QHBoxLayout;
	undo_clear_box->addWidget(new CalcButton("undo", 'u', this));
	undo_clear_box->addWidget(new CalcButton("clear", 'c', this));
	buttons->addLayout(undo_clear_box, 0, 2, 1, 3);
	// row 2
	buttons->addWidget(new CalcButton("7", '7', this), 1, 0);
	buttons->addWidget(new CalcButton("8", '8', this), 1, 1);
	buttons->addWidget(new CalcButton("9", '9', this), 1, 2);
	buttons->addWidget(new CalcButton("^", '^', this), 1, 3);
	buttons->addWidget(new CalcButton("÷", 'd', this), 1, 4);
	// row 3
	buttons->addWidget(new CalcButton("4", '4', this), 2, 0);
	buttons->addWidget(new CalcButton("5", '5', this), 2, 1);
	buttons->addWidget(new CalcButton("6", '6', this), 2, 2);
	buttons->addWidget(new CalcButton("log", 'l', this), 2, 3);
	buttons->addWidget(new CalcButton("×", 'x', this), 2, 4);
	// row 4
	buttons->addWidget(new CalcButton("1", '1', this), 3, 0);
	buttons->addWidget(new CalcButton("2", '2', this), 3, 1);
	buttons->addWidget(new CalcButton("3", '3', this), 3, 2);
	buttons->addWidget(new CalcButton("mod", 'm', this), 3, 3);
	buttons->addWidget(new CalcButton("−", '-', this), 3, 4);
	// row 5
	buttons->addWidget(new CalcButton("₁₀^", 'e', this), 4, 0);
	buttons->addWidget(new CalcButton("0", '0', this), 4, 1);
	buttons->addWidget(new CalcButton(".", '.', this), 4, 2);
	buttons->addWidget(new CalcButton("±", 's', this), 4, 3);
	buttons->addWidget(new CalcButton("+", '+', this), 4, 4);
	// row 6
	buttons->addWidget(new CalcButton("√x", 'r', this), 5, 0);
	buttons->addWidget(new CalcButton("1/x", 'i', this), 5, 1);
	buttons->addWidget(new CalcButton("x!", '!', this), 5, 2);
	buttons->addWidget(new CalcButton("=", 'q', this), 5, 3, 1, 2);
	
	//-----------------------overall layout------------------------
	QLabel *hline = new QLabel(this);
	hline->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	hline->setFixedHeight(5);
	hline->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
									 QSizePolicy::Maximum));
	
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addLayout(displays);
	vbox->addWidget(hline);
	vbox->addLayout(buttons);
	vbox->setSizeConstraint(QLayout::SetFixedSize);
	setLayout(vbox);
}

//----------------------------destructor-----------------------------

Calculator::~Calculator() {
	print_all_events();
}

//------------------------------getters------------------------------

// public getters for viewing state
QString Calculator::get_upper_text() {
	return upper_display->text();
}

QString Calculator::get_lower_text() {
	return lower_display->text();
}

QString Calculator::get_memory1() {
	return memory1;
}

QString Calculator::get_memory2() {
	return memory2;
}

char Calculator::get_binary_op() {
	return cur_binary_op;
}

//-----------------------------do_event------------------------------

// an extremely simple exception class that handles bad states
// literally just a thing I can throw
class BadStateError{};

// calls an input function based on event, also sets add_next_event
// returns whether the event was recognized by the switch statement
bool Calculator::do_event(const char event, bool add_this_event) {
	try {
		switch (event) {
			case '0' ... '9':
			case '.':
				on_digit(event);
				break;
			case '+':
			case '-':
			case 'x':
			case 'd':
			case '^':
			case 'l':
			case 'm':
				on_binary(event);
				break;
			case 'r':
			case 'i':
			case '!':
				on_unary(event);
				break;
			case 'M':
			case 'W':
				on_memory(event);
				break;
			case 'e':
				on_scientific();
				break;
			case 's':
				on_sign();
				break;
			case 'q':
				on_equals();
				break;
			case 'c':
				on_clear();
				break;
			case 'u':
				on_undo();
				break;
			default:
				return false;
		}
	}
	catch (const BadStateError &error) {
		add_this_event = false;
	}
	if (add_this_event)
		add_event(event);
	return true;
}

// handles key press events, passes on to QWidget if not recognized
void Calculator::keyPressEvent(QKeyEvent *event) {
	// convert from text to char
	QChar intermediate;
	if (!event->text().isEmpty())
		intermediate = event->text()[0];
	if (intermediate != 'M') // necessary because 'm' and 'M' are different
		intermediate = intermediate.toLower();
	char event_char = intermediate.toLatin1();
	
	// handle keys different from the event chars
	switch (event->key()) {
		// functions
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_Equal:
			event_char = 'q';
			break;
		case Qt::Key_Backspace:
		case Qt::Key_Delete:
		case Qt::Key_Z:
			event_char = 'u';
			break;
		case Qt::Key_Escape:
			event_char = 'c';
			break;
			// memory
		case Qt::Key_ParenLeft:
		case Qt::Key_BraceLeft:
		case Qt::Key_BracketLeft:
			event_char = 'M';
			break;
		case Qt::Key_ParenRight:
		case Qt::Key_BraceRight:
		case Qt::Key_BracketRight:
		case Qt::Key_W: // overrides toLower() because I use uppercase W
		case Qt::Key_N: // symmetry is _N_ice
			event_char = 'W';
			break;
			// dot
		case Qt::Key_Comma:
			event_char = '.';
			break;
			// binary ops
		case Qt::Key_Slash:
			event_char = 'd';
			break;
		case Qt::Key_Asterisk:
			event_char = 'x';
			break;
			// unary ops
		case Qt::Key_F:
			event_char = '!';
			break;
	}
	bool key_recognized = do_event(event_char, true);
	if (!key_recognized)
		QWidget::keyPressEvent(event);
}

//--------------------------input functions--------------------------

// adds a digit to the active display, also handles decimal points
// will replace the current display if overwrite is set
void Calculator::on_digit(const char digit) {
	QString active_str = active_display->text();
	if (digit == '0' && active_str == "0")
		throw BadStateError();
	
	// handle overwriting the display
	if (overwrite_on_input) {
		overwrite_on_input = (digit == '0');
		active_has_error = false;
		active_str = (digit == '.') ? "0" : "";
	} else {
		if (at_max_precision(active_str))
			throw BadStateError();
		if ((digit == '0') && 
			(active_str.endsWith("e+") || active_str.endsWith("e-"))) {
			throw BadStateError();
		} else if ((digit == '.') && 
			(active_str.contains(".") || active_str.contains("e"))) {
			throw BadStateError();
		}
	}
	active_display->setText(active_str + digit);
}

// changes the current binary op to the pressed one, does no calculations
// sets the active display to lower display, initializes it if not set
// initializing the lower display triggers overwrite
void Calculator::on_binary(const char binary_op) {
	// maps binary event chars to their visual string representation
	static const QMap<char, QString> binary_strings = { 
		{'+', "+"}, {'-', "−"}, {'x', "×"}, {'d', "÷"}, {'^', "^"}, 
		{'l', "log"}, {'m', "mod"}
	};
	if (active_has_error)
		throw BadStateError();
	binary_display->setText(binary_strings.value(binary_op));
	cur_binary_op = binary_op;
	
	if (active_display == upper_display) {
		active_display = lower_display;
		overwrite_on_input = true;
		lower_display->setText("0");
	}
}

// calculates the value of the unary op applied to the display value
// replaces the display value with the calculated value
// triggers overwrite, can trigger active_has_error
void Calculator::on_unary(const char unary_op) {
	if (active_has_error)
		throw BadStateError();
	QString new_value;
	try {
		double value = string_to_double(active_display->text());
		check_unary_error(unary_op, value);
		value = calculate_unary(unary_op, value);
		new_value = double_to_string(value);
		check_number_error(new_value);
	}
	catch (const QString &error_message) {
		active_has_error = true;
		new_value = error_message;
	}
	overwrite_on_input = true;
	active_display->setText(new_value);
}

// either writes memory to the display or reads the display value into memory
// writing to the display triggers overwrite
void Calculator::on_memory(const char mem) {
	QString &mem_str = (mem == 'M') ? memory1 : memory2;
	QString active_str = active_display->text();
	
	if (active_str != "0" && !active_has_error) {
		mem_str = active_str;
	} else if (!mem_str.isEmpty()) {
		overwrite_on_input = true;
		active_has_error = false;
		active_display->setText(mem_str);
	} else {
		throw BadStateError();
	}
	update_memory_display();
}

// adds the scientific notation character 'e+' to the active display
// can append 'e+' to an overwrite value if it didn't have it before
void Calculator::on_scientific() {
	if (active_has_error)
		throw BadStateError();
	QString active_str = active_display->text();
	if (active_str.contains("e") || 
		string_to_double(active_str) == 0.0) {
		throw BadStateError();
	}
	// scientific has a unique overwrite reaction
	// it allows a number to append a new exponent even if it was calculated
	overwrite_on_input = false;
	active_display->setText(active_str + "e+");
}

// swaps the sign of the number or if the number has e, the sign of e
// will still swap if overwrite is set
void Calculator::on_sign() {
	if (active_has_error) 
		throw BadStateError();
	QString active_str = active_display->text();
	if (active_str == "0")
		throw BadStateError();
	
	if (active_str.contains("e+"))
		active_str.replace("e+", "e-");
	else if (active_str.contains("e-"))
		active_str.replace("e-", "e+");
	else if (active_str.startsWith("-"))
		active_str.remove(0, 1);
	else
		active_str.prepend("-");
	
	active_display->setText(active_str);
}

//-------------------------functional inputs-------------------------

// either recalculates the upper display, or does the binary calculation
// clears the display and places the value in the upper display
// triggers overwrite, can trigger active_has_error
void Calculator::on_equals() {
	if (active_has_error)
		throw BadStateError();
	QString new_value;
	try {
		double up, lo, value;
		// either recalculate the upper value or attempt the binary calculation
		value = string_to_double(upper_display->text());
		QString lo_str = lower_display->text();
		if (!lo_str.isEmpty()) {
			up = value;
			lo = string_to_double(lo_str);
			check_binary_error(up, lo);
			value = calculate_binary(up, lo);
		}
		new_value = double_to_string(value);
		check_number_error(new_value);
	}
	catch (const QString &error_message) {
		active_has_error = true;
		new_value = error_message;
	}
	print_state();
	reset_displays(new_value);
	event_frames.append({ new_value, "" });
}

// clears the display and sets upper_display to "0", triggers overwrite
void Calculator::on_clear() {
	print_state();
	reset_displays();
	active_has_error = false;
	event_frames.append({QString("0"), "" });
}

// returns the calculator to the previous state before the most recent event
void Calculator::on_undo() {
	QString &recent_events = event_frames.last().second;
	// remove the last event
	if (recent_events.isEmpty()) {
		if (event_frames.size() > 1)
			event_frames.pop_back();
	} else {
		char last_event = recent_events.back().toLatin1();
		recent_events.chop(1);
		// update old_value variables
		switch (last_event) {
			case 'M':
				old_mem1_values.pop_back();
				all_mem_values.pop_back();
				break;
			case 'W':
				old_mem2_values.pop_back();
				all_mem_values.pop_back();
				break;
			case 'r':
			case 'i':
			case '!':
				old_unary_values.pop_back();
				break;
		}
	}
	reset_state();
}

//-------------------------display functions-------------------------

// clears displays and sets active display to upper
// triggers overwrite flag, but does not alter active_has_error
void Calculator::reset_displays(const QString &reset_val) {
	overwrite_on_input = true;
	active_display = upper_display;
	upper_display->setText(reset_val);
	lower_display->setText("");
	binary_display->setText("");
}

// updates the memory display based on stored memory strings
void Calculator::update_memory_display() {
	QString memory_status = "   ";
	if (memory1.isEmpty())
		memory_status.prepend("◯");
	else
		memory_status.prepend("◉");
	if (memory2.isEmpty())
		memory_status.append("◯");
	else
		memory_status.append("◉");
	memory_display->setText(memory_status);
}

//--------------------------number functions-------------------------

// these handle string conversion
QString Calculator::double_to_string(const double value) {
	return QString().setNum(value, 'g', MAX_PRECISION);
}

double Calculator::string_to_double(const QString &str) {
	bool ok = true;
	double value = str.toDouble(&ok);
	if (ok)
		return value;
	else if (str.endsWith("e+") || str.endsWith("e-"))
		return string_to_double(str.chopped(2));
	else
		return value;
}

// checks if str is at the max precision
bool Calculator::at_max_precision(QString str) {
	int exp_pos = str.indexOf('e');
	if (exp_pos != -1) {
		// compare the length of the numbers starting after e
		return (str.length() - (exp_pos + 2)) >= EXP_PRECISION;
	} else {
		// remove non significant features, then compare the length
		str.remove('-');
		if (str.startsWith('0'))
			str.remove(0,1);
		str.remove('.');
		return str.length() >= MAX_PRECISION;
	}
}

// returns the result of cur_binary_op applied to up and lo
double Calculator::calculate_binary(const double up, const double lo) {
	switch (cur_binary_op) {
		case '+':
			return up + lo;
		case '-':
			return up - lo;
		case 'x':
			return up * lo;
		case 'd':
			return up / lo;
		case '^':
			return std::pow(up, lo);
		case 'l': // log base up of lo
			return std::log(lo) / std::log(up);
		case 'm':
			return std::fmod(up, lo);
	}
	return -420;
}

// because cmath doesn't have a factorial function
// only defined to 20!
long long factorial(int n) {
	if ( 0 > n || n >= 21)
		return -1;
	long long fact = 1;
	for (int i=1; i <= n; ++i)
		fact *= i;
	return fact;
}

// returns the result of unary_op applied to value
double Calculator::calculate_unary(const char unary_op, const double value) {
	switch (unary_op) {
		case 'r':
			return std::sqrt(value);
		case '!':
			return factorial(value);
		case 'i':
			return 1.0 / value;
	}
	return -69;
}

//--------------------------error checkers---------------------------
// these all throw a QString containing the error message if an error is found
// all error messages contain the string "error" in them
// these functions are solely called by on_equals() and on_unary()
// which wrap them in try catch blocks to handle the error messages

// checks for errors regarding invalid inputs to the binary operator
void Calculator::check_binary_error(const double up, const double lo) {
	switch (cur_binary_op) {
		case '^':
			if (up == 0 && lo == 0)
				throw QString("0^0 error");
			else if (up < 0 && std::fmod(lo, 1) != 0)
				throw QString("neg root error");
			break;
		case 'd':
			if (lo == 0)
				throw QString("divide by 0 error");
			break;
		case 'l':
			if (up == 0 || lo == 0)
				throw QString("log 0 error");
			else if (up < 0 || lo < 0)
				throw QString("neg log error");
			break;
		case 'm':
			if (lo == 0)
				throw QString("mod 0 error");
			break;
	}
}

// checks for errors regarding invalid inputs to the unary operator
void Calculator::check_unary_error(const char unary_op, const double value) {
	switch (unary_op) {
		case 'r':
			if (value < 0)
				throw QString("neg root error");
			break;
		case '!':
			if (value < 0)
				throw QString("neg factorial error");
			else if (value >= 21)
				throw QString("factorial size error");
			else if (std::fmod(value, 1) != 0)
				throw QString("dec factorial error");
			break;
		case 'i':
			if (value == 0)
				throw QString("inverse 0 error");
			break;
	}
}

// checks for value equaling inf, -inf, or nan
void Calculator::check_number_error(const QString &value) {
	if (value == "inf")
		throw QString("max size error");
	else if (value == "-inf")
		throw QString("min size error");
	else if (value == "nan")
		throw QString("nan error");
}

//---------------------------undo functions--------------------------

// appends the given event to the event list for the current frame
// also updates old mem values and old unary values
void Calculator::add_event(const char event) {
	switch (event) {
		case 'M':
			old_mem1_values.append(memory1);
			all_mem_values.append(&old_mem1_values.last());
			break;
		case 'W':
			old_mem2_values.append(memory2);
			all_mem_values.append(&old_mem2_values.last());
			break;
		case 'r':
		case 'i':
		case '!':
			old_unary_values.append(active_display->text());
			break;
		case 'q':
		case 'c':
		case 'u':
			return; // don't add functional inputs
	}
	event_frames.last().second += event;
}

// sets the state of the calculator to that of the last event frame
// updates the displays, flags, and memory to reflect the new state
void Calculator::reset_state() {
	reset_displays(event_frames.last().first);
	QString &recent_events = event_frames.last().second;
	int unary_size = old_unary_values.size();
	int mem_size = all_mem_values.size();
	
	int first_binary_pos = recent_events.indexOf(VALID_BINARY);
	QString upper_events = recent_events.left(first_binary_pos);
	// only set the lower and binary displays if there was a binary operator
	if (first_binary_pos == -1) {
		reset_active_display(upper_events, unary_size, mem_size);
	} else {
		// set the upper, binary, then lower displays
		QString lower_events = recent_events.mid(first_binary_pos);
		
		int unary_offset = unary_size - lower_events.count(VALID_UNARY);
		int mem_offset = mem_size - lower_events.count(VALID_MEM);
		reset_active_display(upper_events, unary_offset, mem_offset);
		
		int last_binary_pos = recent_events.lastIndexOf(VALID_BINARY);
		on_binary(recent_events[last_binary_pos].toLatin1());
		// note the active display is now lower
		reset_active_display(lower_events, unary_size, mem_size);
	}
	// update memory values
	if (old_mem1_values.isEmpty())
		memory1.clear();
	else
		memory1 = old_mem1_values.last();
	if (old_mem2_values.isEmpty())
		memory2.clear();
	else
		memory2 = old_mem2_values.last();
	update_memory_display();
}

// requires active_display has been set to the default value
// takes display_events, finds the most recent event that triggered overwrite,
// updates the display to that point, then redoes all events after that point
void Calculator::reset_active_display(QString &display_events, 
									  const int unary_offset, 
									  const int mem_offset) {
	display_events.remove(VALID_BINARY);
	int last_unary_pos = display_events.lastIndexOf(VALID_UNARY);
	int index = 0;
	// find the last overwrite event, default is already accounted for
	if (last_unary_pos != -1) {
		index = last_unary_pos + 1;
		active_display->setText(old_unary_values.at(unary_offset - 1));
	} else if (display_events[0] == 'M' || display_events[0] == 'W') {
		index = 1;
		int mem_pos = mem_offset - display_events.count(VALID_MEM);
		active_display->setText(*all_mem_values.at(mem_pos));
	}
	active_has_error = active_display->text().contains("error");
	// redo the events starting from index
	for (int i = index; i < display_events.size(); ++i) {
		do_event(display_events[i].toLatin1(), false);
	}
}

//----------------------------debuggers------------------------------

// prints recent events, current displays, flags, and mem values
// called in on_clear() and on_equals()
void Calculator::print_state() {
	out.setRealNumberPrecision(MAX_PRECISION);
	if (!event_frames.isEmpty()) {
		out << "\n---info---"
		<< "\nenter val:\t" << event_frames.last().first
		<< "\nevents:\t" << event_frames.last().second;
		out << "\n--displays--"
		<< "\nupper:\t" << upper_display->text()
		<< "\nbinary:\t" << binary_display->text()
		<< "\nlower:\t" << lower_display->text();
		out << "\n---flags---" 
		<< "\noverwrite:\t" << overwrite_on_input
		<< "\nhas_error:\t" << active_has_error;
		out << "\n--memory--"
		<< "\n1:\t" << memory1
		<< "\n2:\t" << memory2;
		out << '\n';
	}
}

// prints past event frames and the values of old mem and old unary
// called in the destructor
void Calculator::print_all_events() {
	out << "\nevent list:";
	foreach(auto pair, event_frames) {
		out << "\n  val:    " << pair.first;
		out << "\n  events: " << pair.second;
	}
	out << "\n\nunary values:\n  ";
	foreach(QString str, old_unary_values) {
		out << str << ", ";
	}
	out << "\nmem1 values:\n  ";
	foreach(QString str, old_mem1_values) {
		out << str << ", ";
	}
	out << "\nmem2 values:\n  ";
	foreach(QString str, old_mem2_values) {
		out << str << ", ";
	}
	out << '\n';
}

