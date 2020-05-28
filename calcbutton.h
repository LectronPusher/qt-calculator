#pragma once

#include <QPushButton>

class CalcButton : public QPushButton {
	Q_OBJECT
	
public:
	char event_char;
	
	// sets text, event_char, size policy, and connects clicked to func_on_click
	CalcButton(const QString &text_in, const char& event_char_in,
			   QWidget *parent, const char *func_on_click)
	: QPushButton(parent), event_char(event_char_in) {
		setText(text_in);
		setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
								  QSizePolicy::MinimumExpanding));
		connect(this, SIGNAL(clicked()), parent, func_on_click);
	}
	
	// make it slightly taller and square if not expanding
	QSize sizeHint() const {
		QSize size = QPushButton::sizeHint();
		size.rheight() += 3;
		size.rwidth() = size.rheight();
		return size;
	}
	
};


