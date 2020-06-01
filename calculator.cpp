#include "calculator.h"
#include "calclabel.h"
#include "calcbutton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

#include <cmath>

// for debugging
#include <QTextStream>
static QTextStream out(stdout);

//----------------------------constructor----------------------------

Calculator::Calculator(QWidget *parent) 
	: QWidget(parent),
	VALID_BINARY("[+\\-xd^lm]"),
	VALID_UNARY("[ri!]"),
	VALID_MEM("[MW]")
{	
	// setup keyboard focus
	setFocusPolicy(Qt::StrongFocus);
	
	//----------------------display widgets------------------------
	
	upper_display = new CalcLabel(true, this);
	lower_display = new CalcLabel(true, this);
	binary_display = new CalcLabel(false, this);
	memory_display = new CalcLabel(false, this);
	memory_display->setFrameStyle(QFrame::NoFrame);
	
	QGridLayout *display = new QGridLayout;
	display->addWidget(memory_display, 0, 0);
	display->addWidget(upper_display, 0, 1);
	display->addWidget(binary_display, 1, 0);
	display->addWidget(lower_display, 1, 1);
	
	// set the displays to their initial states
	active_has_error = false;
	reset_displays();
	update_memory_display();
	
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
	
	//----------------------layout management----------------------
	
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
		case Qt::Key_Space:
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
		// memory
		case Qt::Key_ParenLeft:
		case Qt::Key_BraceLeft:
		case Qt::Key_BracketLeft:
			event_char = 'M';
			break;
		case Qt::Key_ParenRight:
		case Qt::Key_BraceRight:
		case Qt::Key_BracketRight:
		case Qt::Key_W: // overrides intermediate because I use uppercase W
		case Qt::Key_N: // symmetry is nice
			event_char = 'W';
			break;
	}
	bool key_recognized = do_event(event_char, true);
	if (!key_recognized)
		QWidget::keyPressEvent(event);
}


//-----------------------------do_event------------------------------

bool Calculator::do_event(const char event, bool add_this_event) {
	add_next_event = add_this_event;
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
			out << "unidentified event\n";
			return false;
	}
	return true;
}

//----------------------------input slots----------------------------

void Calculator::on_digit(const char digit) {
	QString active_str = active_display->text();
	
	// handle overwriting the display
	if (overwrite_on_input) {
		overwrite_on_input = (digit == '0');
		active_has_error = false;
		active_str = (digit == '.') ? "0" : "";
	} else {
		// ignore input if too precise, or for specific exceptions
		if (at_max_precision(active_str))
			return;
		if ((digit == '0') && 
			(active_str.endsWith("e+") || active_str.endsWith("e-"))) {
			return;
		} else if ((digit == '.') && 
			(active_str.contains(".") || active_str.contains("e"))) {
			return;
		}
	}
	active_display->setText(active_str + digit);
	add_event(digit);
}

void Calculator::on_binary(const char binary_op) {
	// maps binary event chars to their visual string representation
	static const QMap<char, QString> binary_strings = { 
		{'+', "+"}, {'-', "−"}, {'x', "×"}, {'d', "÷"}, {'^', "^"}, 
		{'l', "log"}, {'m', "mod"}};
	if (active_has_error)
		return;
	binary_display->setText(binary_strings.value(binary_op));
	cur_binary_op = binary_op;
	
	if (active_display == upper_display) {
		active_display = lower_display;
		overwrite_on_input = true;
		lower_display->setText("0");
	}
	add_event(binary_op);
}

void Calculator::on_unary(const char unary_op) {
	if (active_has_error)
		return;
	QString active_str = active_display->text();
	if (active_str.isEmpty())
		return;
	
	double value = string_to_double(active_str);
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
		return;
	}
	update_memory_display();
	add_event(mem);
}

void Calculator::on_scientific() {
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
	add_event('e');
}

void Calculator::on_sign() {
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
	add_event('s');
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

void Calculator::on_undo() {
	QString &recent_events = event_frames.last().second;
	// remove the last event
	if (recent_events.isEmpty()) {
		if (event_frames.size() > 1)
			event_frames.pop_back();
	} else {
		QChar last_event = recent_events.back();
		recent_events.chop(1);
		// update old_value variables
		if (VALID_MEM.match(last_event).hasMatch()) {
			all_mem_values.pop_back();
			((last_event == 'M') ? old_mem1_values : old_mem2_values).pop_back();
		} else if (VALID_UNARY.match(last_event).hasMatch()) {
			old_unary_values.pop_back();
		}
	}
	reset_state();
}

//-------------------------display functions-------------------------

void Calculator::reset_displays(const QString &reset_val) {
	if (add_next_event) {
		print_state();
		event_frames.append({ reset_val, "" });
	}
	overwrite_on_input = true;
	active_display = upper_display;
	upper_display->setText(reset_val);
	lower_display->setText("");
	binary_display->setText("");
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

bool Calculator::at_max_precision(QString str) {
	int exp_pos = str.indexOf('e');
	if (exp_pos != -1) {
		// just the length of the numbers starting after e
		return str.length() - (exp_pos + 2) >= EXP_PRECISION;
	} else {
		// remove non significant feautures, then just count length
		str.remove('-');
		if (str.startsWith('0'))
			str.remove(0,1);
		str.remove('.');
		return str.length() >= MAX_PRECISION;
	}
}

double Calculator::calculate_binary(const double up, const double lo) {
	double result;
	switch (cur_binary_op) {
		case '+':
			result = up + lo;
			break;
		case '-':
			result = up + -lo;
			break;
		case 'x':
			result = up * lo;
			break;
		case 'd':
			result = up / lo;
			break;
		case '^':
			result = std::pow(up, lo);
			break;
		case 'l':
			result = std::log(lo) / std::log(up); // log base up of lo
			break;
		case 'm':
			result = std::fmod(up, lo);
			break;
		default:
			out << "binary op error\n";
			result = -420;
			break;
	}
	return result;
}

long long Calculator::factorial(int n) {
	if ( 0 > n || n >= 21)
		return -1;
	long long fact = 1;
	for (int i=1; i <= n; ++i)
		fact *= i;
	return fact;
}

double Calculator::calculate_unary(const char unary_op, const double value) {
	double result;
	switch (unary_op) {
		case 'r':
			result = std::sqrt(value);
			break;
		case '!':
			result = factorial(value);
			break;
		case 'i':
			result = 1.0 / value;
			break;
		default:
			out << "unary op error\n";
			result = -69;
			break;
	}
	return result;
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
			else if (value >= 21)
				error_message = "factorial size error";
			else if (std::fmod(value, 1) != 0)
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

QString Calculator::check_number_error(const QString &value) {
	QString error_message;
	if (value == "inf")
		error_message = "max size error";
	else if (value == "-inf")
		error_message = "min size error";
	else if (value == "nan")
		error_message = "nan error";
	if (!error_message.isEmpty()) {
		active_has_error = true;
		overwrite_on_input = true;
		return error_message;
	} else
		return value;
}

//---------------------------undo functions--------------------------

void Calculator::add_event(const char event) {
	if (add_next_event) {
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
		event_frames.last().second += event;
	}
}

void Calculator::reset_state() {
	add_next_event = false;
	reset_displays(event_frames.last().first);
	QString &recent_events = event_frames.last().second;
	int unary_size = old_unary_values.size();
	int mem_size = all_mem_values.size();
	
	// split the events at the first binary
	int binary_pos = recent_events.indexOf(VALID_BINARY);
	QString upper_events = recent_events.left(binary_pos);
	
	// if there was no binary, only set the upper display
	if (binary_pos == -1) {
		set_display(upper_events, unary_size, mem_size);
	} else {
		// reset the upper, binary, then lower displays
		QString lower_events = recent_events.mid(binary_pos);
		
		int unary_offset = unary_size - lower_events.count(VALID_UNARY);
		int mem_offset = mem_size - lower_events.count(VALID_MEM);
		set_display(upper_events, unary_offset, mem_offset);
		
		int last_binary_pos = recent_events.lastIndexOf(VALID_BINARY);
		on_binary(recent_events[last_binary_pos].toLatin1());
		// note the active display is now lower
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

void Calculator::set_display(QString &display_events, const int unary_offset, 
							 const int mem_offset) {
	// only deals with the active display
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

