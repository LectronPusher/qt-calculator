#include "calculator.h"
#include "calclabel.h"
#include "calcbutton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

#include <cmath>

#include <QTextStream>
static QTextStream out(stdout);

//----------------------------constructor----------------------------

Calculator::Calculator(QWidget *parent) : QWidget(parent) {
	// necessary for on_binary functionality
	binary_strings.insert('+', "+");
	binary_strings.insert('-', "−");
	binary_strings.insert('x', "×");
	binary_strings.insert('d', "÷");
	binary_strings.insert('^', "^");
	binary_strings.insert('l', "log");
	binary_strings.insert('m', "mod");
	
	out.setRealNumberPrecision(MAX_PRECISION);
	
	//-------------------label widgets-------------------
	
	upper_display = new CalcLabel("", true, this);
	upper_display->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	active_display = upper_display;
	
	lower_display = new CalcLabel("", true, this);
	lower_display->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	
	binary_display = new CalcLabel("", false, this);
	binary_display->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	
	memory_display = new CalcLabel("", false, this);
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
	CalcButton *clear = new CalcButton("clear", '\0', this, SLOT(on_clear()));
	CalcButton *undo = new CalcButton("undo", '\0', this, SLOT(on_undo()));
	CalcButton *equals = new CalcButton("=", '\0', this, SLOT(on_equals()));
	
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
// end constructor


//----------------------------input slots----------------------------

void Calculator::on_digit(char digit) {
	digit = event_char_from_button(sender(), digit);
	// get the active str, modify it, then set it later
	QString active_str = active_display->text();
	// digit should definitely be defined for overwrites
	if (overwrite_on_input) {
		overwrite_on_input = false;
		active_has_error = false;
		active_str.clear();
	}
	// don't allow getting too precise, I'm only using doubles here
	if (exponent_length(active_str) >= EXP_PRECISION)
		return;
	else if (num_sig_figs(active_str) >= MAX_PRECISION)
		return;
	// ignore input for these states
	switch (digit) {
		case '0':
			if ((active_str.startsWith("0") && !active_str.contains(".")) || 
				(active_str.startsWith("-0") && !active_str.contains(".")) || 
				active_str.endsWith("e+") || 
				active_str.endsWith("e-"))
				return;
			break;
		case '.':
			if (active_str.contains(".") || 
				active_str.contains("e"))
				return;
			break;
	}
	// update active display and event list
	active_display->setText(active_str + digit);
	add_event(digit);
}

void Calculator::on_scientific(char scien) {
	// get the active str, modify it, then set it later
	QString active_str = active_display->text();
	// ignore input for bad states
	if (active_has_error)
		return;
	if (active_str.isEmpty() || 
		active_str.contains("e") || 
		active_str == "-" || 
		active_str == "." || 
		active_str == "-.")
		return;
	// update active display and event list
	active_display->setText(active_str + "e+");
	add_event(event_char_from_button(sender(), scien));
	// scientific has a unique overwrite reaction
	// it allows a number to have its exponent be modified
	overwrite_on_input = false;
}

void Calculator::on_sign(char sign) {
	// get the active str, modify it, then set it later
	QString active_str = active_display->text();
	// ignore input for bad states
	if (active_has_error) 
		return;
	// the big if else statement that controls the galaxy
	if (active_str.contains("e+"))
		active_str.replace("e+", "e-");
	else if (active_str.contains("e-"))
		active_str.replace("e-", "e+");
	else if (active_str.startsWith("-"))
		active_str.remove(0, 1);
	else
		active_str.prepend("-");
	// update active display and event list
	active_display->setText(active_str);
	add_event(event_char_from_button(sender(), sign));
}

void Calculator::on_binary(char binary_op) {
	binary_op = event_char_from_button(sender(), binary_op);
	// ignore input for bad states
	if (active_has_error)
		return;
	if (upper_display->text().isEmpty())
		return;
	// update the display and other variables
	active_display = lower_display;
	binary_display->setText(binary_strings[binary_op]);
	cur_binary_op = binary_op;
	add_event(binary_op);
}

void Calculator::on_memory(char mem) {
	mem = event_char_from_button(sender(), mem);
	// get the active str, modify it, then set it later
	QString active_str = active_display->text();
	// change the corresponding memory string
	QString &mem_str = (mem == 'M') ? memory1 : memory2;
	// ignore input for bad states
	if (active_has_error)
		return;
	// bad states passed, do logic
	if (!active_str.isEmpty())
		mem_str = active_str;
	else if (!mem_str.isEmpty()) {
		// setting active causes overwrite
		overwrite_on_input = true;
		active_display->setText(mem_str);
	} else
		return;
	// store old memory values for undo
	QStringList &old_mem = (mem == 'M') ? old_mem1_values : old_mem2_values;
	old_mem.push_back(mem_str);
	// update memory display and event list
	update_memory_display();
	add_event(mem);
}

void Calculator::on_unary(char unary_op) {
	unary_op = event_char_from_button(sender(), unary_op);
	// get the active str, modify it, then set it later
	QString active_str = active_display->text();
	// ignore input for bad states
	if (active_has_error)
		return;
	if (active_str.isEmpty())
		return;
	// bad states passed, do calculation
	double value = string_to_double(active_str);
	if (check_unary_error(unary_op, value))
		return;
	// calculate the new value
	value = calculate_unary(unary_op, value);
	QString new_value = double_to_string(value);
	new_value = check_number_error(new_value);
	// update active display and event list
	active_display->setText(new_value);
	add_event(unary_op);
	// returning a calculated unary value causes overwrite
	overwrite_on_input = true;
}


//-------------------------functional slots--------------------------

void Calculator::on_clear() {
	active_has_error = false;
	clear_displays();
}

void Calculator::on_equals() {
	QString up_str = upper_display->text();
	QString lo_str = lower_display->text();
	bool just_recalculate_upper = lo_str.isEmpty();
	// ignore certain bad states
	if (active_has_error)
		return;
	if (up_str.isEmpty())
		return;
	if (just_recalculate_upper && !binary_display->text().isEmpty())
		return;
	// bad states passed
	
	double up = string_to_double(up_str);
	double lo =  (just_recalculate_upper) ? 0 : string_to_double(lo_str);
	double value;
	// either do the binary calculation or just recalculate the upper value
	if (just_recalculate_upper) {
		value = string_to_double(up_str);
	} else {
		if (check_binary_error(up, lo))
			return;
		value = calculate_binary(up, lo);
	}
	QString new_value = double_to_string(value);
	new_value = check_number_error(new_value);
	// do debug printing
	out << "events: ";
	foreach(std::string str, event_list[event_list.size() - 1])
		out << str.c_str() << " ";
	out << '\n';
	out << "upper:  " << up_str << "  " << up << '\n';
	out << "lower:  " << lo_str << "  " << lo << '\n';
	out << "dbl: " << value << '\n';
	out << "str: " << new_value << "\n\n";
	
	clear_displays(new_value);
}


//-------------------------display functions-------------------------

void Calculator::set_active_input(const bool set_to_lower) {
	if (set_to_lower)
		active_display = lower_display;
	else
		active_display = upper_display;
}

void Calculator::clear_displays(QString new_upper_str) {
	event_list.push_back({ new_upper_str.toStdString(), "" });
	upper_display->setText(new_upper_str);
	lower_display->setText("");
	binary_display->setText("");
	overwrite_on_input = true;
	// does not alter active_has_error
	active_display = upper_display;
}

void Calculator::update_memory_display() {
	QString new_status = "   ";
	if (memory1.isEmpty())
		new_status.prepend("◯");
	else
		new_status.prepend("◉");
	if (memory2.isEmpty())
		new_status.append("◯");
	else
		new_status.append("◉");
	memory_display->setText(new_status);
}


//--------------------------number functions-------------------------

QString Calculator::double_to_string(const double value) {
	QString str;
	str.setNum(value, 'g', MAX_PRECISION);
	return str;
}

double Calculator::string_to_double(const QString str) {
	bool ok = true;
	double val = str.toDouble(&ok);
	if (ok)
		return val;
	else {
		if (str == ".")
			return 0;
		else if (str == "-")
			return 0;
		else if (str == "-.")
			return 0;
		else if (str.endsWith("e+"))
			return string_to_double(str.chopped(2));
		else if (str.endsWith("e-"))
			return string_to_double(str.chopped(2));
		else {
			out << "string to double error\n";
			return val;
		}
	}
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
		return str.length() - (exp_pos + 2/*offset*/);
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
			value = -420;
			break;
	}
	return value;
}

//--------------------------error checkers---------------------------

bool Calculator::check_binary_error(const double up, const double lo) {
	QString error_message;
	switch (cur_binary_op) {
		case '^': // power
			if (up == 0 && lo == 0)
				error_message = "0^0 error";
			else if (up < 0 && std::fmod(lo, 1) != 0)
				error_message = "neg power error";
			break;
		case 'd': // divide
			if (lo == 0)
				error_message = "divide by 0 error";
			break;
		case 'l': // log
			if (up == 0 || lo == 0)
				error_message = "log 0 error";
			else if (up < 0 || lo < 0)
				error_message = "neg log error";
			break;
		case 'm': // mod
			if (lo == 0)
				error_message = "mod 0 error";
			break;
	}
	if (!error_message.isEmpty()) {
		active_has_error = true;
		overwrite_on_input = true;
		clear_displays(error_message);
		return true;
	}
	return false;
}

bool Calculator::check_unary_error(const char unary_op, const double value) {
	QString error_message;
	switch (unary_op) {
		case 'r': // root
			if (value < 0)
				error_message = "neg root error";
			break;
		case '!': // factorial
			if (value < 0)
				error_message = "neg factorial error";
			if (value > 20)
				error_message = "factorial size error";
			if (std::fmod(value, 1) != 0)
				error_message = "dec factorial error";
			break;
		case 'i': // inverse
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
	QString error;
	if (value == "inf")
		error = "maximum size error";
	else if (value == "-inf")
		error = "minimum size error";
	else if (value == "nan")
		error = "nan error";
	if (!error.isEmpty()) {
		active_has_error = true;
		overwrite_on_input = true;
		return error;
	} else
		return value;
}


//---------------------------undo functions--------------------------

void Calculator::on_undo() {
	print_events();
	// TODO
}

void Calculator::add_event(const char event) {
	if (to_add_event) {
		switch (event) {
			case 'M':
				old_mem1_values.push_back(memory1);
				break;
			case 'W':
				old_mem2_values.push_back(memory2);
				break;
			case 'r':
			case 'i':
			case '!':
				old_unary_values.push_back(active_display->text());
				break;
		}
		event_list.last().last() += event;
	}
}

char Calculator::event_char_from_button(QObject *button, char input_char) {
	if ((to_add_event = !input_char))
		input_char = qobject_cast<CalcButton *>(button)->event_char;
	return input_char;
}

void Calculator::print_events() {
	out << "\nevent list:\n";
	foreach(QList<std::string> list, event_list) {
		out << "\tval:    " << list.first().c_str() << '\n';
		out << "\tevents: " << list.last().c_str() << '\n';
	}
	out << "\n flags:\n" 
		<< "\toverwrite_on_input = " << overwrite_on_input << '\n'
		<< "\tactive_has_error   = " << active_has_error << '\n';
	
	out << "\nold mem1 values:\n";
	foreach(QString str, old_mem1_values) {
		out << str << ", ";
	}
	out << "\nold mem2 values:\n";
	foreach(QString str, old_mem2_values) {
		out << str << ", ";
	}
	out << "\nold unary values:\n";
	foreach(QString str, old_unary_values) {
		out << str << ", ";
	}
}

