#pragma once

#include <Qt/qlabel.h>
#include <Engine/Tools/QT/LinkedSlider.h>
#include <ExportHeader.h>
#include <sstream>

namespace DebugMenuControllers {
	struct ENGINE_SHARED CharPointerController {
		QLabel * label;
		const char * title;
		const char ** data;

		CharPointerController() {
			label = new QLabel();
		}
		inline void init(const char * name, const char ** toWatch) {
			title = name;
			data = toWatch;
			update();
		}
		inline void update() {
			std::stringstream ss;
			ss << title << *data;
			label->setText(QString( ss.str().c_str() ));
		}
	};
}