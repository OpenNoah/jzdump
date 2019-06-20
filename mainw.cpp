#include <iostream>
#include <noahmobile/nmqmainwindow.h>
#include <noahmobile/nmqmessagebox.h>
#include <noahmobile/nmqwidget.h>
#include <opie2/oapplicationfactory.h>
#include <opie2/oapplication.h>
#include <qt.h>

int main(int argc, char *argv[])
{
	Opie::Core::OApplication a(argc, argv);
	NMQMessageBox::information(0, "Test", "Text");
	return a.exec();
}
