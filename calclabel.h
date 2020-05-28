#pragma once

#include <QLabel>
// #include <QTextStream>

//  static QTextStream out(stdout);

class CalcLabel : public QLabel {
	Q_OBJECT
	
public:
	CalcLabel (const QString &text_in, bool main_display, 
			   QWidget *parent = 0)
	: QLabel(parent) {
		setText(text_in);
		setTextFormat(Qt::PlainText);
		
		QFont new_font = font();
		new_font.setPointSize(new_font.pointSize() + 1);
		setFont(new_font);
		
		if (main_display) {
			setAlignment(Qt::AlignRight);
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
