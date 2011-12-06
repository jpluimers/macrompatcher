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
	void selectDiskImage();
	void splitImage();

private:
	void updateChecksumUI();

	QAction *openAction;
	QAction *saveAction;
	QAction *exitAction;
	QAction *splitAction;

	QMenu *fileMenu;
	QMenu *toolsMenu;

	QLabel *checksum;
	QCheckBox *applyRomdisk;
	QLineEdit *romdiskFile;
	QPushButton *romdiskSelect;

	RomCtx *rom;
};
