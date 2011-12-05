#include "RomPatcher.h"

RomPatcher::RomPatcher()
{
	openAction = new QAction(tr("&Open"), this);
	saveAction = new QAction(tr("&Save"), this);
	exitAction = new QAction(tr("&Exit"), this);

	connect(openAction, SIGNAL(triggered()), this, SLOT(open()));
	connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));
	connect(exitAction, SIGNAL(triggered()), this, SLOT(quit()));

	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(openAction);
	fileMenu->addAction(saveAction);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAction);

	int mywidth = 300;
	int myheight = 300;
	int yoffset = 25;
	int xoffset = 5;
	checksum = new QLabel(this);
	checksum->setText("Checksum: <NA>");
	checksum->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	checksum->move(xoffset, yoffset);
	yoffset += 25;
	checksum->setMinimumSize(mywidth, 10);

	applyRomdisk = new QRadioButton("Apply ROMdisk Driver", this);
	applyRomdisk->move(xoffset, yoffset);
	yoffset += 25;
	applyRomdisk->setMinimumSize(mywidth, 10);
	applyRomdisk->setEnabled(false);

	romdiskFile = new QLineEdit(this);
	romdiskFile->move(xoffset, yoffset);
	romdiskFile->setMinimumSize(mywidth-110, 5);
	romdiskFile->setEnabled(false);

	romdiskSelect = new QPushButton("ROMdisk Image", this);
	romdiskSelect->move(mywidth-105, yoffset);
	romdiskSelect->setMinimumSize(100, 5);
	yoffset += 25;
	connect(romdiskSelect, SIGNAL(clicked()), this, SLOT(selectDiskImage()));
	romdiskSelect->setEnabled(false);

	setWindowTitle(tr("RomPatcher"));
	setMinimumSize(mywidth, myheight);

	rom = (RomCtx*)calloc(1, sizeof(RomCtx));
}

void RomPatcher::open()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", "");
	if(fileName != "") {
		QFile file(fileName);
		if(!file.open(QIODevice::ReadOnly)) {
			QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
			return;
		}

		// create the RomCtx structure and populate it here
		rom->datasize = rom->filesize = file.size();
		rom->data = (uint8_t*)calloc(1, rom->datasize);
		QDataStream stream(&file);
		stream.readRawData((char*)rom->data, (uint)rom->datasize);
		file.close();

		if(rom->filesize < (512*1024)) {
			rom->type = e24bit;
		}else{
			rom->type = e32bit;
		}

		updateChecksumUI();
		applyRomdisk->setEnabled(true);
		romdiskFile->setEnabled(true);
		romdiskSelect->setEnabled(true);
	}
}

void RomPatcher::save()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", "");
	if(fileName != "") {
		QFile file(fileName);
		if(!file.open(QIODevice::WriteOnly)) {
			QMessageBox::critical(this, tr("Error"), tr("Could not save file"));
			return;
		}

		// save the RomCtx structure here
		applyMods();

		RomErr err = UpdateChecksum(rom);
		if(err != eSuccess) {
			fprintf(stderr, "Error updating checksum: %d %s\n", err, GetROMErrString(err));
			QMessageBox::critical(this, "Error", "Could not update checksum");
		}

		QString romdiskimagename = romdiskFile->text();
		if(romdiskimagename != "") {
			QFile imagefile(romdiskimagename);
			if(!imagefile.open(QIODevice::ReadOnly)) {
				QMessageBox::critical(this, "Error", "Could not open ROMdisk Image");
				return;
			}

			if(imagefile.size() != (512*1024)) {
				QMessageBox::critical(this, "Error", "ROMdisk Image is the wrong size");
				return;
			}

			uint8_t *image = (uint8_t*)calloc(1, imagefile.size());
			QDataStream imagestream(&imagefile);
			imagestream.readRawData((char*)image, (int)imagefile.size());
			imagefile.close();
			err = InstallRomdiskImage(rom, image, (uint32_t)imagefile.size());
			if(err) {
				QMessageBox::critical(this, "Error", "ROMdisk Image couldn't be installed");
				return;
			}
		}
		
		QDataStream stream(&file);
		stream.writeRawData((char*)rom->data, (int)rom->datasize);

		file.close();
	}
}

void RomPatcher::quit()
{
	exit(0);
}

void RomPatcher::updateChecksumUI()
{
	uint32_t cksum;
	char *cksumstr = NULL;
	GetChecksum(rom, &cksum);
        checksum->setText(QString().sprintf("%#x", cksum));
	free(cksumstr);
}

void RomPatcher::applyMods()
{
	if(applyRomdisk->isChecked()) {
		printf("Romdisk checked, applying\n");
		RomErr err = InstallRomdiskDrvr(rom);
		if(err != eSuccess) {
			fprintf(stderr, "Error applying romdisk drvr: %d %s\n", err, GetROMErrString(err));
		}
	}

	updateChecksumUI();
}

void RomPatcher::selectDiskImage()
{
	QString s = QFileDialog::getOpenFileName(this, "Select ROMdisk Image", "./", "All Files (*.*)");
	romdiskFile->setText(s);
}
