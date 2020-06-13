#pragma once

#include <QLabel>

class CalcLabel : public QLabel {
public:
	// change certain layout features and visual aesthetics
	CalcLabel (bool number_display, QWidget *parent) : QLabel(parent) {
		// make the font slightly bigger
		QFont new_font = font();
		new_font.setPointSize(new_font.pointSize() + 1);
		setFont(new_font);
		setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
		
		if (number_display) {
			setSizePolicy(QSizePolicy(QSizePolicy::Expanding, 
									  QSizePolicy::Fixed));
			setAlignment(Qt::AlignRight);
			setTextInteractionFlags(Qt::TextSelectableByMouse);
		}
		else {
			setSizePolicy(QSizePolicy(QSizePolicy::Minimum, 
									  QSizePolicy::Fixed));
			setAlignment(Qt::AlignCenter);
		}
	}
	
	QSize sizeHint() const {
		QSize size = QLabel::sizeHint();
		size.rwidth() = 50;
		return size;
	}
};
