#pragma once

#include <QLabel>
// #include <QTextStream>

//  static QTextStream out(stdout);

class CalcLabel : public QLabel {
	Q_OBJECT
	
public:
	CalcLabel (bool number_display, QWidget *parent = 0) : QLabel(parent) {
		setTextFormat(Qt::PlainText);
		
		QFont new_font = font();
		new_font.setPointSize(new_font.pointSize() + 1);
		setFont(new_font);
		
		if (number_display) {
			setAlignment(Qt::AlignRight);
			setTextInteractionFlags(Qt::TextSelectableByMouse);
			setSizePolicy(QSizePolicy(QSizePolicy::Expanding, 
									  QSizePolicy::Fixed));
		}
		else {
			setAlignment(Qt::AlignCenter);
			setSizePolicy(QSizePolicy(QSizePolicy::Minimum, 
									  QSizePolicy::Fixed));
		}
	}
	
	QSize sizeHint() const {
		QSize size = QLabel::sizeHint();
		size.rwidth() = 50;
// 		out << text() <<  " " << size.rwidth() << '\n';
		return size;
	}
};
