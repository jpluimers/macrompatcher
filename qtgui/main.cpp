#include <QtGui>
#include "RomPatcher.h"

int main(int argc, char **argv) {
	QApplication app(argc, argv);

	RomPatcher rp;

	rp.show();

	return app.exec();
}
