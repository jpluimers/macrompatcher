#include <QtGui>
#include "../lib/macrompatcher.h"

class RomPatcher : public QMainWindow
{
	Q_OBJECT

public:
	RomPatcher();

private slots:
	void open();
	void save();
	void quit();
	void applyMods();

private:
	void updateChecksumUI();

	QAction *openAction;
	QAction *saveAction;
	QAction *exitAction;

	QMenu *fileMenu;

	QLabel *checksum;
	QRadioButton *applyRomdisk;

	RomCtx *rom;
};
