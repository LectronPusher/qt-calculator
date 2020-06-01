#pragma once

#include "calculator.h"
#include <QPushButton>

class CalcButton : public QPushButton {
	Q_OBJECT
	
	Calculator *calc;
	char event_char;
	
public:
	// does setup, allows button to call calc->do_event when clicked
	CalcButton(const QString &text_in, const char& event, Calculator *parent)
	: QPushButton(parent), calc(parent), event_char(event) {
		setText(text_in);
		connect(this, SIGNAL(clicked()), this, SLOT(send_event_to_calc()));
		setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
								  QSizePolicy::MinimumExpanding));
	}
	
	// make the button slightly taller and square if not expanding
	QSize sizeHint() const {
		QSize size = QPushButton::sizeHint();
		size.rheight() += 3;
		size.rwidth() = size.rheight();
		return size;
	}
	
private slots:
	void send_event_to_calc() {
		calc->do_event(event_char, true);
	}
};


