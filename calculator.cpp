#include "calculator.h"
#include "calclabel.h"
#include "calcbutton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

#include <cmath>

// for debugging
#include <QTextStream>
static QTextStream out(stderr);

//----------------------------constructor----------------------------

Calculator::Calculator(QWidget *parent) 
	: QWidget(parent),
	  VALID_DIGIT("[0-9.]"),
	VALID_SCIEN("[e]"),
	VALID_SIGN("[s]"),
	VALID_BINARY("[+\\-xd^lm]"),
	VALID_UNARY("[ri!]"),
	VALID_MEM("[MW]")
{
	out.setRealNumberPrecision(MAX_PRECISION);
	
	// necessary for on_binary functionality
	binary_strings.insert('+', "+");
	binary_strings.insert('-', "−");
	binary_strings.insert('x', "×");
	binary_strings.insert('d', "÷");
	binary_strings.insert('^', "^");
	binary_strings.insert('l', "log");
	binary_strings.insert('m', "mod");
	
	//-------------------label widgets-------------------
	
	upper_display = new CalcLabel(true, this);
	upper_display->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	
	lower_display = new CalcLabel(true, this);
	lower_display->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	
	binary_display = new CalcLabel(false, this);
	binary_display->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	
	memory_display = new CalcLabel(false, this);
	
	overwrite_on_input = true;
	reset_displays();
	update_memory_display();
	
	//------------------button widgets-------------------
	// digits, includes dot
	CalcButton *dot = new CalcButton(".", '.', this, SLOT(on_digit()));
	CalcButton *zero = new CalcButton("0", '0', this, SLOT(on_digit()));
	CalcButton *one = new CalcButton("1", '1', this, SLOT(on_digit()));
	CalcButton *two = new CalcButton("2", '2', this, SLOT(on_digit()));
	CalcButton *three = new CalcButton("3", '3', this, SLOT(on_digit()));
	CalcButton *four = new CalcButton("4", '4', this, SLOT(on_digit()));
	CalcButton *five = new CalcButton("5", '5', this, SLOT(on_digit()));
	CalcButton *six = new CalcButton("6", '6', this, SLOT(on_digit()));
	CalcButton *seven = new CalcButton("7", '7', this, SLOT(on_digit()));
	CalcButton *eight = new CalcButton("8", '8', this, SLOT(on_digit()));
	CalcButton *nine = new CalcButton("9", '9', this, SLOT(on_digit()));
	// unary operators
	CalcButton *inverse = new CalcButton("1/x", 'i', this, SLOT(on_unary()));
	CalcButton *factorial = new CalcButton("x!", '!', this, SLOT(on_unary()));
	CalcButton *root = new CalcButton("√x", 'r', this, SLOT(on_unary()));
	// binary operators
	CalcButton *plus = new CalcButton("+", '+', this, SLOT(on_binary()));
	CalcButton *minus = new CalcButton("−", '-', this, SLOT(on_binary()));
	CalcButton *times = new CalcButton("×", 'x', this, SLOT(on_binary()));
	CalcButton *divide = new CalcButton("÷", 'd', this, SLOT(on_binary()));
	CalcButton *power = new CalcButton("^", '^', this, SLOT(on_binary()));
	CalcButton *log = new CalcButton("log", 'l', this, SLOT(on_binary()));
	CalcButton *mod = new CalcButton("mod", 'm', this, SLOT(on_binary()));
	// unique but not functions
	CalcButton *scien = new CalcButton("₁₀^", 'e', this, SLOT(on_scientific()));
	CalcButton *sign = new CalcButton("±", 's', this, SLOT(on_sign()));
	CalcButton *mem1 = new CalcButton("M1", 'M', this, SLOT(on_memory()));
	CalcButton *mem2 = new CalcButton("M2", 'W', this, SLOT(on_memory()));
	// functions
	CalcButton *equals = new CalcButton("=", '\0', this, SLOT(on_equals()));
	CalcButton *clear = new CalcButton("clear", '\0', this, SLOT(on_clear()));
	CalcButton *undo = new CalcButton("undo", '\0', this, SLOT(on_undo()));
	
	//-----------------layout management-----------------
	
	QGridLayout *display = new QGridLayout;
	display->addWidget(memory_display, 0, 0);
	display->addWidget(upper_display, 0, 1);
	display->addWidget(binary_display, 1, 0);
	display->addWidget(lower_display, 1, 1);
	
	QGridLayout *buttons = new QGridLayout;
	
	buttons->addWidget(mem1, 0, 0);
	buttons->addWidget(mem2, 0, 1);
	QHBoxLayout *clear_undo = new QHBoxLayout;
	clear_undo->addWidget(undo);
	clear_undo->addWidget(clear);
	buttons->addLayout(clear_undo, 0, 2, 1, 3);
	
	buttons->addWidget(seven, 1, 0);
	buttons->addWidget(eight, 1, 1);
	buttons->addWidget(nine, 1, 2);
	buttons->addWidget(power, 1, 3);
	buttons->addWidget(divide, 1, 4);
	
	buttons->addWidget(four, 2, 0);
	buttons->addWidget(five, 2, 1);
	buttons->addWidget(six, 2, 2);
	buttons->addWidget(log, 2, 3);
	buttons->addWidget(times, 2, 4);
	
	buttons->addWidget(one, 3, 0);
	buttons->addWidget(two, 3, 1);
	buttons->addWidget(three, 3, 2);
	buttons->addWidget(mod, 3, 3);
	buttons->addWidget(minus, 3, 4);
	
	buttons->addWidget(scien, 4, 0);
	buttons->addWidget(zero, 4, 1);
	buttons->addWidget(dot, 4, 2);
	buttons->addWidget(sign, 4, 3);
	buttons->addWidget(plus, 4, 4);
	
	buttons->addWidget(root, 5, 0);
	buttons->addWidget(inverse, 5, 1);
	buttons->addWidget(factorial, 5, 2);
	buttons->addWidget(equals, 5, 3, 1, 2);
	
	QLabel *hline = new QLabel(this);
	hline->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	hline->setFixedHeight(5);
	hline->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
									 QSizePolicy::Maximum));
	
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addLayout(display);
	vbox->addWidget(hline);
	vbox->addLayout(buttons);
	setLayout(vbox);
}

//----------------------------destructor-----------------------------

Calculator::~Calculator() {
	print_all_events();
}

//----------------------------input slots----------------------------

void Calculator::on_digit(char digit) {
	digit = event_char_from_button(sender(), digit);
	QString active_str = active_display->text();
	
	// we have some weird stuff here due to the display needing to start with 0
	// also the 0 and dot else ifs are bugging me but I cannot get it any better
	if (overwrite_on_input) {
		overwrite_on_input = (digit == '0');
		active_has_error = false;
		active_str = (digit == '.') ? "0" : "";
	} else if ((digit == '0') && (
		active_str.endsWith("e+") || 
		active_str.endsWith("e-"))) {
		return;
	} else if ((digit == '.') && (
		active_str.contains(".") || 
		active_str.contains("e"))) {
		return;
	}
	// don't allow getting too precise, I'm only using doubles here
	if (active_str.contains("e")) {
		if (exponent_length(active_str) >= EXP_PRECISION)
			return;
	} else if (num_sig_figs(active_str) >= MAX_PRECISION) {
		return;
	}
	active_display->setText(active_str + digit);
	add_event(digit);
}

void Calculator::on_scientific(char scien) {
	if (active_has_error)
		return;
	QString active_str = active_display->text();
	if (active_str.contains("e") || 
		string_to_double(active_str) == 0.0) {
		return;
	}
	// scientific has a unique overwrite reaction
	// it allows a number to append a new exponent even if it was calculated
	overwrite_on_input = false;
	active_display->setText(active_str + "e+");
	add_event(event_char_from_button(sender(), scien));
}

void Calculator::on_sign(char sign) {
	if (active_has_error) 
		return;
	QString active_str = active_display->text();
	if (active_str == "0")
		return;
	
	if (active_str.contains("e+"))
		active_str.replace("e+", "e-");
	else if (active_str.contains("e-"))
		active_str.replace("e-", "e+");
	else if (active_str.startsWith("-"))
		active_str.remove(0, 1);
	else
		active_str.prepend("-");
	
	active_display->setText(active_str);
	add_event(event_char_from_button(sender(), sign));
}

void Calculator::on_memory(char mem) {
	mem = event_char_from_button(sender(), mem);
	QString &mem_str = (mem == 'M') ? memory1 : memory2;
	QString active_str = active_display->text();
	
	if (active_str != "0" && !active_has_error) {
		mem_str = active_str;
	} else if (!mem_str.isEmpty()) {
		overwrite_on_input = true;
		active_has_error = false;
		active_display->setText(mem_str);
	} else {
		return;
	}
	update_memory_display();
	add_event(mem);
}

void Calculator::on_binary(char binary_op) {
	if (active_has_error)
		return;
	binary_op = event_char_from_button(sender(), binary_op);
	binary_display->setText(binary_strings[binary_op]);
	cur_binary_op = binary_op;
	
	if (active_display == upper_display) {
		active_display = lower_display;
		overwrite_on_input = true;
		lower_display->setText("0");
	}
	add_event(binary_op);
}

void Calculator::on_unary(char unary_op) {
	if (active_has_error)
		return;
	QString active_str = active_display->text();
	if (active_str.isEmpty())
		return;
	
	double value = string_to_double(active_str);
	unary_op = event_char_from_button(sender(), unary_op);
	// we still add the event if there was an error
	// the error func handles the display by itself
	if (!check_unary_error(unary_op, value)) {
		value = calculate_unary(unary_op, value);
		QString new_value = double_to_string(value);
		new_value = check_number_error(new_value);
		
		// returning a calculated unary value causes overwrite
		overwrite_on_input = true;
		active_display->setText(new_value);
	}
	add_event(unary_op);
}

//-------------------------functional slots--------------------------

void Calculator::on_equals() {
	if (active_has_error)
		return;
	QString up_str = upper_display->text();
	QString lo_str = lower_display->text();
	
	// either recalculate the upper value or attempt the binary calculation
	double up, lo, value;
	up = string_to_double(up_str);
	if (binary_display->text().isEmpty()) {
		value = string_to_double(up_str);
	} else {
		lo = string_to_double(lo_str);
		if (check_binary_error(up, lo))
			return;
		value = calculate_binary(up, lo);
	}
	QString new_value = double_to_string(value);
	new_value = check_number_error(new_value);
	reset_displays(new_value);
}

void Calculator::on_clear() {
	reset_displays();
	active_has_error = false;
}

//-------------------------display functions-------------------------

void Calculator::reset_displays(QString new_upper_str, bool to_add_event) {
	print_state();
	overwrite_on_input = true;
	active_display = upper_display;
	upper_display->setText(new_upper_str);
	lower_display->setText("");
	binary_display->setText("");
	if (to_add_event)
		event_list.append({ new_upper_str, "" });
}

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

QString Calculator::double_to_string(const double value) {
	return QString().setNum(value, 'g', MAX_PRECISION);
}

double Calculator::string_to_double(const QString str) {
	bool ok = true;
	double value = str.toDouble(&ok);
	if (ok)
		return value;
	else if (str.endsWith("e+") || str.endsWith("e-"))
		return string_to_double(str.chopped(2));
	else
		return value;
}

int Calculator::num_sig_figs(QString str) {
	int exp_pos = str.indexOf('e');
	if (exp_pos != -1)
		str.truncate(exp_pos);
	str.remove('-');
	if (str.startsWith('0'))
		str.remove(0,1);
	str.remove('.');
	return str.length();
}

int Calculator::exponent_length(QString str) {
	int exp_pos = str.indexOf('e');
	if (exp_pos == -1)
		return 0;
	else 
		return str.length() - (exp_pos + /*offset*/2);
}

double Calculator::calculate_binary(const double up, const double lo) {
	double value;
	switch (cur_binary_op) {
		case '+':
			value = up + lo;
			break;
		case '-':
			value = up + -lo;
			break;
		case 'x':
			value = up * lo;
			break;
		case 'd':
			value = up / lo;
			break;
		case '^':
			value = std::pow(up, lo);
			break;
		case 'l':
			value = std::log(lo) / std::log(up); // log base up of lo
			break;
		case 'm':
			value = std::fmod(up, lo);
			break;
		default:
			out << "binary op error\n";
			value = -420;
			break;
	}
	return value;
}

// really? no factorial function in cmath? boooo
// only defined up to 20!
long long factorial(int n) {
	if ( 0 > n || n >= 21)
		return -1;
	long long fact = 1;
	for (int i=1; i <= n; ++i)
		fact *= i;
	return fact;
}

double Calculator::calculate_unary(const char unary_op, double value) {
	switch (unary_op) {
		case 'r':
			value = std::sqrt(value);
			break;
		case '!':
			value = factorial(value);
			break;
		case 'i':
			value = 1.0 / value;
			break;
		default:
			out << "unary op error\n";
			value = -69;
			break;
	}
	return value;
}

//--------------------------error checkers---------------------------

bool Calculator::check_binary_error(const double up, const double lo) {
	QString error_message;
	switch (cur_binary_op) {
		case '^':
			if (up == 0 && lo == 0)
				error_message = "0^0 error";
			else if (up < 0 && std::fmod(lo, 1) != 0)
				error_message = "neg root error";
			break;
		case 'd':
			if (lo == 0)
				error_message = "divide by 0 error";
			break;
		case 'l':
			if (up == 0 || lo == 0)
				error_message = "log 0 error";
			else if (up < 0 || lo < 0)
				error_message = "neg log error";
			break;
		case 'm':
			if (lo == 0)
				error_message = "mod 0 error";
			break;
	}
	if (!error_message.isEmpty()) {
		active_has_error = true;
		overwrite_on_input = true;
		reset_displays(error_message);
		return true;
	}
	return false;
}

bool Calculator::check_unary_error(const char unary_op, const double value) {
	QString error_message;
	switch (unary_op) {
		case 'r':
			if (value < 0)
				error_message = "neg root error";
			break;
		case '!':
			if (value < 0)
				error_message = "neg factorial error";
			if (value >= 21)
				error_message = "factorial size error";
			if (std::fmod(value, 1) != 0)
				error_message = "dec factorial error";
			break;
		case 'i':
			if (value == 0)
				error_message = "inverse 0 error";
			break;
	}
	if (!error_message.isEmpty()) {
		active_has_error = true;
		overwrite_on_input = true;
		active_display->setText(error_message);
		return true;
	}
	return false;
}

QString Calculator::check_number_error(QString value) {
	QString error_message;
	if (value == "inf")
		error_message = "maximum size error";
	else if (value == "-inf")
		error_message = "minimum size error";
	else if (value == "nan")
		error_message = "nan error";
	if (!error_message.isEmpty()) {
		active_has_error = true;
		overwrite_on_input = true;
		return error_message;
	} else
		return value;
}

//----------------------------debuggers------------------------------

void Calculator::print_state() {
	if (!event_list.isEmpty()) {
		out << "\n---info---"
		<< "\nenter val:\t" << event_list.last().first
		<< "\nevents:\t" << event_list.last().second;
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

void Calculator::print_all_events() {
	out << "\nevent list:";
	foreach(auto pair, event_list) {
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

//---------------------------undo functions--------------------------

void Calculator::add_event(const char event) {
	if (to_add_event) {
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
		}
		event_list.last().second += event;
	}
}

char Calculator::event_char_from_button(QObject *button, char input_char) {
	to_add_event = (input_char == '\0');
	if (to_add_event)
		input_char = qobject_cast<CalcButton *>(button)->event_char;
	return input_char;
}

void Calculator::on_undo() {
	QString &recent_events = event_list.last().second;
	// remove the last event
	if (recent_events.isEmpty()) {
		if (event_list.size() > 1)
			event_list.pop_back();
	} else {
		char last_event = recent_events.back().toLatin1();
		recent_events.chop(1);
		// update old_value variables
		if (VALID_MEM_STR.contains(last_event)) {
			all_mem_values.pop_back();
			((last_event == 'M') ? old_mem1_values : old_mem2_values).pop_back();
		} else if (VALID_UNARY_STR.contains(last_event)) {
			old_unary_values.pop_back();
		}
	}
	reset_state();
}

void Calculator::reset_state() {
	reset_displays(event_list.last().first, false);
	QString &recent_events = event_list.last().second;
	int unary_size = old_unary_values.size();
	int mem_size = all_mem_values.size();
	
	// split the events at the first binary
	int binary_pos = recent_events.indexOf(VALID_BINARY);
	QString upper_events = recent_events.left(binary_pos);
	
	// if there was no binary, only set the upper display
	if (binary_pos == -1) {
		set_display(upper_events, unary_size, mem_size);
	} else {
		QString lower_events = recent_events.mid(binary_pos);
		
		int unary_offset = unary_size - lower_events.count(VALID_UNARY);
		int mem_offset = mem_size - lower_events.count(VALID_MEM);
		set_display(upper_events, unary_offset, mem_offset);
		
		// reset the last pressed binary
		int last_binary_pos = recent_events.lastIndexOf(VALID_BINARY);
		on_binary(recent_events[last_binary_pos].toLatin1());
		// this simplifies the set_display logic
		lower_events.remove(VALID_BINARY);
		set_display(lower_events, unary_size, mem_size);
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

void Calculator::set_display(const QString &display_events, int unary_offset,
							 int mem_offset) {
	int last_unary_pos = display_events.lastIndexOf(VALID_UNARY);
	int index = 0;
	// find the last overwrite event, default is already accounted for
	if (last_unary_pos != -1) {
		index = last_unary_pos + 1;
		active_display->setText(old_unary_values.at(unary_offset - 1));
	} else if (VALID_MEM_STR.contains(display_events[0])) {
		index = 1;
		int mem_pos = mem_offset - display_events.count(VALID_MEM);
		active_display->setText(*all_mem_values.at(mem_pos));
	}
	active_has_error = active_display->text().contains("error");
	// redo the events starting from index
	for (int i = index; i < display_events.size(); ++i) {
		redo_event(display_events[i].toLatin1());
	}
}

void Calculator::redo_event(char event) {
	if (VALID_DIGIT_STR.contains(event))
		on_digit(event);
	else if (VALID_SCIEN_STR.contains(event))
		on_scientific(event);
	else if (VALID_SIGN_STR.contains(event))
		on_sign(event);
	else if (VALID_MEM_STR.contains(event))
		on_memory(event);
	else if (VALID_BINARY_STR.contains(event))
		on_binary(event);
	else if (VALID_UNARY_STR.contains(event))
		on_unary(event);
	else
		out << "unidentified event\n";
}

