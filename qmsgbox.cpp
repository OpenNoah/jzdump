#include <stdio.h>
#include <stdlib.h>
#include <noahmobile/nmqmainwindow.h>
#include <noahmobile/nmqmessagebox.h>
#include <noahmobile/nmqwidget.h>
#include <opie2/oapplication.h>
#include <qmessagebox.h>
#include <qtextcodec.h>

int main(int argc, char *argv[])
{
#if 0
	char lock = 0;
	int i;
	for (i = 1; i < argc; i++)
		if (strcasecmp(argv[i], "-qws"))
			lock = 1;
	if (lock)
		system("/opt/QtPalmtop/bin/qcop QPE/System \"setMouseProto(QString)\" \"TPanel:none\"");
#endif
	Opie::Core::OApplication a(argc , argv);
	QTextCodec *codec = QTextCodec::codecForName("GB2312");
	a.setDefaultCodec(codec);
	if (argc == 1)
		//printf("%d", QMessageBox(NULL, QObject::tr(argv[5]), QObject::tr(argv[6]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4])));
		printf("Qt-msgbox v3.0 (2019-06-20)\n\n"
		"Usage: qmsgbox [type] [[[button code 1 - 3] | [button text 1 - 3] [default button] [cancel button]] [title] [content]]\n\n"
		"Displays a brief message, an icon and some buttons\n\n"
		"Options:\n"
		"	type		Kind of dialog to use\n"
		"	button code	Code for predefined buttons\n"
		"	button text	Text on the buttons\n"
		"	default button	Default button for text buttons\n"
		"	cancel button	Cancel button for text buttons\n"
		"	title		Title text\n"
		"	content		Content texts\n\n"
		"Details:\n"
		"	Dialog types:\n"
		"	  None	This help message\n"
		"		eg. qmsgbox\n"
		"	  1	Information dialog with predefined buttons\n"
		"		eg. qmsgbox 1 %d %d %d \"Qt\" \"Some texts...\"\n"
		"	  11	Information dialog with customise text buttons\n"
		"		eg. qmsgbox 11 \"&One\" \"T&wo\" \"Three\" 2 1 \"Info\" \"Select:\"\n"
		"	  2	Warning dialog with predefined buttons\n"
		"	  12	Warning dialog with customise text buttons\n"
		"	  3	Critical dialog with predefined buttons\n"
		"	  13	Critical dialog with customise text buttons\n"
		"	  4	About dialog with no button options\n"
		"		eg. qmsgbox 4 \"About\" \"This software made by zhiyb.\"\n"
		"	  5	About Qt dialog with no options\n"
		"		eg. qmsgbox 5\n"
		"	Predefined button codes:\n"
		"	  NoButton	%d\n"
		"	  Ok		%d\n"
		"	  Cancel	%d\n"
		"	  Yes		%d\n"
		"	  No		%d\n"
		"	  Abort		%d\n"
		"	  Retry		%d\n"
		"	  Ignore	%d\n"
		"	  Default	+%d\n"
		"	  Escape	+%d\n\n"
		"Return:\n"
		"	Display the index of the selected button.\n"
		"	You can just simply use $(qmsgbox ...) or `qmsgbox ...` (if you just has based shell) to get the index.\n"
		"	You may need to know that the index for buttons is sorted from 0 to 2.\n"
		"	Notice: the index for `default button' and `cancel button' also same as this, from 0 to 2.\n",\
			QMessageBox::Ok + QMessageBox::Default, QMessageBox::NoButton, QMessageBox::NoButton,\
			QMessageBox::NoButton, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::Yes, QMessageBox::No, QMessageBox::Abort, QMessageBox::Retry, QMessageBox::Ignore, QMessageBox::Default, QMessageBox::Escape);
	else if (atoi(argv[1]) == 1)
		printf("%d", QMessageBox::information(NULL, QObject::tr(argv[5]), QObject::tr(argv[6]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4])));
	else if (atoi(argv[1]) == 11)
		printf("%d", QMessageBox::information(NULL, QObject::tr(argv[7]), QObject::tr(argv[8]), QObject::tr(argv[2]), QObject::tr(argv[3]), QObject::tr(argv[4]), atoi(argv[5]), atoi(argv[6])));
	else if (atoi(argv[1]) == 2)
		printf("%d", QMessageBox::warning(NULL, QObject::tr(argv[5]), QObject::tr(argv[6]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4])));
	else if (atoi(argv[1]) == 12)
		printf("%d", QMessageBox::warning(NULL, QObject::tr(argv[7]), QObject::tr(argv[8]), QObject::tr(argv[2]), QObject::tr(argv[3]), QObject::tr(argv[4]), atoi(argv[5]), atoi(argv[6])));
	else if (atoi(argv[1]) == 3)
		printf("%d", QMessageBox::critical(NULL, QObject::tr(argv[5]), QObject::tr(argv[6]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4])));
	else if (atoi(argv[1]) == 13)
		printf("%d", QMessageBox::critical(NULL, QObject::tr(argv[7]), QObject::tr(argv[8]), QObject::tr(argv[2]), QObject::tr(argv[3]), QObject::tr(argv[4]), atoi(argv[5]), atoi(argv[6])));
	else if (atoi(argv[1]) == 4)
		QMessageBox::about(NULL, QObject::tr(argv[2]), QObject::tr(argv[3]));
	else if (atoi(argv[1]) == 5)
		QMessageBox::aboutQt(NULL);
#if 0
	if (lock)
		system("/opt/QtPalmtop/bin/qcop QPE/System \"setMouseProto(QString)\" \"TPanel:/dev/jz_ts\"");
#endif
	return 0;
}
