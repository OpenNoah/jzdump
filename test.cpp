#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <algorithm>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <noahmobile/nmqmessagebox.h>
#include "test.h"
#include "ahb_regs.h"

#define TR(s)	QObject::tr(s)

#define AHB_BASE	0x13000000
#define AHB_SIZE	0x00090000

#define APB_BASE	0x10000000
#define APB_SIZE	0x00074000
#define GPIO_BASE	0x10010000
#define GPIO_SIZE	0x00001000

#define _I	volatile
#define _O	volatile
#define _IO	volatile

#pragma packed(push, 1)
struct gpio_t {
	struct gpio_port_t {
		_I uint32_t D;
		_O uint32_t S;
		_O uint32_t C;
			uint32_t _RESV;
	} PIN, DAT, IM, PE, FUN, SEL, DIR, TRG, FLG;
};
#pragma packed(pop)

static struct sys_t {
	int mem;
	void *ahb, *apb;
} sys;

static struct test_info_t {
	std::string dir;
	int success, total, finished;
	int timer;
	bool testKeys;
	struct gpio_info_t {
		static const uint32_t known[4];
		uint32_t mask[4];
		uint32_t ignored[4];
		uint32_t row[4], column[4];
		uint32_t input[4], pin[4], output[4];
	} gpio;
} testInfo;

// Known or problematic ports
const uint32_t test_info_t::gpio_info_t::known[4] = {
	0x00000000,
	0x00000000,
	0xc8000000,	// PC31: UART/JTAG, PC30: NAND ready/busy, PC27: MMC power?
	0x00000000,
};

void test(int id)
{
	testInfo.total = std::max(15, id);	// TODO
	testInfo.timer = 100;
	int i = 0;
	if (id == i++) {
		testInfo.finished = 0;
		testInfo.success = 0;
		ui.updateStatus(TR("开始测试?"));
		if (NMQMessageBox::information(ui.top, TR("jzdump"), TR("确定要开始系统信息采集吗?"),
			QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes) {
			qApp->quit();
			return;
		}
	} else if (id == i++) {
		ui.updateStatus(TR("创建临时目录..."));
		char name[] = "/mnt/mmc/jzdump_XXXXXX";
#if 1
		mkdtemp(name);
#endif
		testInfo.dir = std::string(name);
	} else if (id == i++) {
		ui.updateStatus(TR("转储 AHB 寄存器..."));
#if 0
#if 1
		NMQMessageBox::warning(ui.top, TR("注意"), TR("AHB 采集容易崩溃, 暂时跳过"));
		return;
#endif
		std::string fname("ahb.dump");
		std::ofstream fs((testInfo.dir + "/" + fname).c_str(), std::ios::binary);
		if (!fs.is_open()) {
			NMQMessageBox::critical(ui.top, TR("错误"), TR("无法创建文件:\n%1\n位于:\n%2").arg(fname.c_str()).arg(testInfo.dir.c_str()));
			return;
		}
		if (!fs.write((char *)sys.ahb, AHB_SIZE)) {
			NMQMessageBox::critical(ui.top, TR("错误"), TR("无法写入文件:\n%1\n位于:\n%2").arg(fname.c_str()).arg(testInfo.dir.c_str()));
			return;
		}
		fs.close();
		sync();
#else
		std::string fname("ahb.txt");
		std::ofstream fs((testInfo.dir + "/" + fname).c_str(), std::ios::binary);
		if (!fs.is_open()) {
			NMQMessageBox::critical(ui.top, TR("错误"), TR("无法创建文件:\n%1\n位于:\n%2").arg(fname.c_str()).arg(testInfo.dir.c_str()));
			return;
		}
		const uint32_t *p = &ahb_regs[0];
		while (*p != 0) {
			fs << "0x" << std::hex << std::setfill('0') << std::setw(8) << *p;
			fs << "\t" << std::dec << *(p + 1);
			fs << "\t0x" << std::hex << std::setfill('0');
			switch (*(p + 1)) {
			case 8:
				fs << std::setw(2) << (uint32_t)*(uint8_t *)((uint32_t)sys.ahb + *p - AHB_BASE);
				break;
			case 16:
				fs << std::setw(4) << (uint32_t)*(uint16_t *)((uint32_t)sys.ahb + *p - AHB_BASE);
				break;
			default:
				fs << std::setw(8) << (uint32_t)*(uint32_t *)((uint32_t)sys.ahb + *p - AHB_BASE);
				break;
			}
			fs << std::endl;
			p += 2;
		}
		if (!fs) {
			NMQMessageBox::critical(ui.top, TR("错误"), TR("写入文件失败:\n%1\n位于:\n%2").arg(fname.c_str()).arg(testInfo.dir.c_str()));
			return;
		}
		fs.close();
		sync();
#endif
	} else if (id == i++) {
		ui.updateStatus(TR("转储 APB 寄存器空间..."));
		std::string fname("apb.dump");
		std::ofstream fs((testInfo.dir + "/" + fname).c_str(), std::ios::binary);
		if (!fs.is_open()) {
			NMQMessageBox::critical(ui.top, TR("错误"), TR("无法创建文件:\n%1\n位于:\n%2").arg(fname.c_str()).arg(testInfo.dir.c_str()));
			return;
		}
		if (!fs.write((char *)sys.apb, APB_SIZE)) {
			NMQMessageBox::critical(ui.top, TR("错误"), TR("无法写入文件:\n%1\n位于:\n%2").arg(fname.c_str()).arg(testInfo.dir.c_str()));
			return;
		}
		fs.close();
		sync();
	} else if (id == i++) {
		ui.updateStatus(TR("保存 GPIO 配置..."));
		std::string fname("gpio.txt");
		std::ofstream fs((testInfo.dir + "/" + fname).c_str());
		if (!fs.is_open()) {
			NMQMessageBox::critical(ui.top, TR("错误"), TR("无法创建文件:\n%1\n位于:\n%2").arg(fname.c_str()).arg(testInfo.dir.c_str()));
			return;
		}
		for (int ip = 0; ip < 4; ip++) {
			gpio_t *p = (gpio_t *)((uint8_t *)sys.apb + GPIO_BASE - APB_BASE + 0x100 * ip);
			fs << "Port " << (char)('A' + ip) << ":" << std::endl;
			fs << "PIN = 0x" << std::hex << std::setfill('0') << std::setw(8) << p->PIN.D << std::endl;
			fs << "DAT = 0x" << std::hex << std::setfill('0') << std::setw(8) << p->DAT.D << std::endl;
			fs << "IM  = 0x" << std::hex << std::setfill('0') << std::setw(8) << p->IM.D  << std::endl;
			fs << "PE  = 0x" << std::hex << std::setfill('0') << std::setw(8) << p->PE.D  << std::endl;
			fs << "FUN = 0x" << std::hex << std::setfill('0') << std::setw(8) << p->FUN.D << std::endl;
			fs << "SEL = 0x" << std::hex << std::setfill('0') << std::setw(8) << p->SEL.D << std::endl;
			fs << "DIR = 0x" << std::hex << std::setfill('0') << std::setw(8) << p->DIR.D << std::endl;
			fs << "TRG = 0x" << std::hex << std::setfill('0') << std::setw(8) << p->TRG.D << std::endl;
			fs << "FLG = 0x" << std::hex << std::setfill('0') << std::setw(8) << p->FLG.D << std::endl;
			fs << std::endl;
		}
		if (!fs) {
			NMQMessageBox::critical(ui.top, TR("错误"), TR("写入文件失败:\n%1\n位于:\n%2").arg(fname.c_str()).arg(testInfo.dir.c_str()));
			return;
		}
		fs.close();
		sync();
	} else if (id == i++) {
		ui.updateStatus(TR("查找矩阵键盘驱动 GPIO..."));
		for (int ip = 0; ip < 4; ip++) {
			gpio_t *p = (gpio_t *)((uint8_t *)sys.apb + GPIO_BASE - APB_BASE + 0x100 * ip);
			uint32_t fun = p->FUN.D, sel = p->SEL.D, dir = p->DIR.D, pin = p->PIN.D;
			// Ignore function, interrupt and known pins
			testInfo.gpio.ignored[ip] = fun | sel | testInfo.gpio.known[ip];
			testInfo.gpio.mask[ip]    &= ~testInfo.gpio.ignored[ip];
			// Note input pins
			testInfo.gpio.input[ip]   = testInfo.gpio.mask[ip] & ~fun & ~sel & ~dir;
			testInfo.gpio.pin[ip]     = pin;
			// Change input pins to output with same value
			p->DAT.S = testInfo.gpio.input[ip] & pin;
			p->DAT.C = testInfo.gpio.input[ip] & ~pin;
			p->DIR.S = testInfo.gpio.input[ip];
			// Note output pins
			testInfo.gpio.output[ip]  = testInfo.gpio.mask[ip] & ~fun & ~sel & dir;
			// Change output pins to input
			p->DIR.C = testInfo.gpio.output[ip];
		}
		// Wait for matrix keyboard driver to revert pin directions
		testInfo.timer = 500;
	} else if (id == i++) {
		ui.updateStatus(TR("恢复 GPIO 状态..."));
		for (int ip = 0; ip < 4; ip++) {
			gpio_t *p = (gpio_t *)((uint8_t *)sys.apb + GPIO_BASE - APB_BASE + 0x100 * ip);
			uint32_t dir = p->DIR.D;
			// Revert GPIO direction
			p->DIR.C = testInfo.gpio.input[ip];
			p->DIR.S = testInfo.gpio.output[ip];
			// Only interested in GPIOs that automatically reverted
			testInfo.gpio.input[ip]  &= ~dir;
			testInfo.gpio.output[ip] &= dir;
			testInfo.gpio.mask[ip]   = testInfo.gpio.input[ip] | testInfo.gpio.output[ip];
			std::cout << "Found pins changed on port " << (char)('A' + ip)
					<< " 0x" << std::hex << std::setfill('0') << std::setw(8) << testInfo.gpio.mask[ip] << std::endl;
		}
	} else if (id == i++) {
		ui.updateStatus(TR("禁用键盘..."));
		testInfo.testKeys = NMQMessageBox::warning(ui.top, TR("注意"), TR("之后的测试会自动禁用机身按键,\n请按提示操作"),
								QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok;
		if (!testInfo.testKeys)
			return;
		int err = system("qcop 'QPE/System' 'setKeyboard(QString)' 'None'");
		if (!WIFEXITED(err) || WEXITSTATUS(err) != 0) {
			testInfo.testKeys = false;
			NMQMessageBox::critical(ui.top, TR("错误"), TR("禁用键盘错误: %1 (%2)").arg(WEXITSTATUS(err)).arg(err));
			return;
		}
	} else if (id == i++) {
		ui.updateStatus(TR("测试键盘矩阵 column GPIO..."));
		if (!testInfo.testKeys)
			return;
		NMQMessageBox mb(TR("键盘测试"), TR("请依次按下 *电源键以外* 的\n所有机身按键, 不要同时按下多个,\n完成后请点 OK"),
					QMessageBox::Information, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton,
					ui.top, 0, false);
		mb.show();
		while (!mb.isActiveWindow())
			qApp->processEvents(0);
		bool found = false;
		while (mb.isActiveWindow()) {
			qApp->processEvents(10);
			for (int ip = 0; ip < 4; ip++) {
				if (!(testInfo.gpio.input[ip] & testInfo.gpio.mask[ip]))
					continue;
				gpio_t *p = (gpio_t *)((uint8_t *)sys.apb + GPIO_BASE - APB_BASE + 0x100 * ip);
				uint32_t pin = p->PIN.D;
				uint32_t d = (pin ^ testInfo.gpio.pin[ip]) & testInfo.gpio.input[ip] & testInfo.gpio.mask[ip];
				if (d) {
					std::cout << "Found column pin(s) on port " << (char)('A' + ip)
							<< " 0x" << std::hex << std::setfill('0') << std::setw(8) << d << std::endl;
					testInfo.gpio.column[ip] |= d;
					testInfo.gpio.mask[ip]   &= ~d;
					found = true;
				}
			}
		}

		int result = mb.exec();
		mb.close();
		if (result != 0 || !found) {
			testInfo.testKeys = false;
			if (result == 0)
				NMQMessageBox::critical(ui.top, TR("错误"), TR("没有找到键盘矩阵 column GPIO"));
			return;
		}
	} else if (id == i++) {
		ui.updateStatus(TR("选择一个键盘矩阵 column GPIO..."));
		if (!testInfo.testKeys)
			return;
		NMQMessageBox mb(TR("键盘测试"), TR("请再次依次按下 *电源键以外* 的\n所有机身按键, 直到程序出现提示\n(确定或返回键或许会直接提示)"),
					QMessageBox::Information, QMessageBox::Cancel, QMessageBox::NoButton, QMessageBox::NoButton,
					ui.top, 0, false);
		mb.show();
		while (!mb.isActiveWindow())
			qApp->processEvents(0);
		uint32_t ip = 0, d = 0;
		while (mb.isActiveWindow()) {
			qApp->processEvents(10);
			if (d)
				continue;
			for (ip = 0; ip < 4; ip++) {
				if (!testInfo.gpio.column[ip])
					continue;
				gpio_t *p = (gpio_t *)((uint8_t *)sys.apb + GPIO_BASE - APB_BASE + 0x100 * ip);
				uint32_t pin = p->PIN.D;
				d = (pin ^ testInfo.gpio.pin[ip]) & testInfo.gpio.column[ip];
				if (d) {
					mb.close();
					break;
				}
			}
		}
		if (!d) {
			testInfo.testKeys = false;
			return;
		}

		if (NMQMessageBox::information(ui.top, TR("键盘测试"), TR("请记住这个按键,\n按住这个按键然后点 OK"),
						QMessageBox::Ok, QMessageBox::Cancel) != QMessageBox::Ok) {
			testInfo.testKeys = false;
			return;
		}
		gpio_t *p = (gpio_t *)((uint8_t *)sys.apb + GPIO_BASE - APB_BASE + 0x100 * ip);
		while (!((p->PIN.D ^ testInfo.gpio.pin[ip]) & d)) {
			if (NMQMessageBox::warning(ui.top, TR("键盘测试"), TR("未检测到按键,\n请按住刚才的按键然后点 OK"),
							QMessageBox::Ok, QMessageBox::Cancel) != QMessageBox::Ok) {
				testInfo.testKeys = false;
				return;
			}
		}
	} else if (id == i++) {
		ui.updateStatus(TR("测试键盘矩阵 row GPIO..."));
		if (!testInfo.testKeys)
			return;

		bool found = false;
		for (int ip = 0; ip < 4; ip++) {
			if (testInfo.gpio.output[ip]) {
				if (found) {
					found = false;
					break;
				} else {
					found = true;
				}
			}
		}
		if (found) {
			for (int ip = 0; ip < 4; ip++) {
				uint32_t d = testInfo.gpio.output[ip];
				if (!d)
					continue;
				if (__builtin_popcountl(d) != 1)
					break;
				testInfo.gpio.row[ip]  |= d;
				testInfo.gpio.mask[ip] &= ~d;
				std::cout << "Assuming the single output pin is a row pin, on port " << (char)('A' + ip)
						<< " 0x" << std::hex << std::setfill('0') << std::setw(8) << d << std::endl;
			}
		}

		NMQMessageBox mb(TR("键盘测试"), TR("请按住刚才的按键,\n然后依次按下 *电源键以外* 的\n所有机身按键, 可以同时按下多个,\n完成后请点 OK"),
					QMessageBox::Information, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton,
					ui.top, 0, false);
		mb.show();
		while (!mb.isActiveWindow())
			qApp->processEvents(0);
		found = false;
		while (mb.isActiveWindow()) {
			qApp->processEvents(10);
			for (int ip = 0; ip < 4; ip++) {
				if (!(testInfo.gpio.input[ip] & testInfo.gpio.mask[ip]))
					continue;
				gpio_t *p = (gpio_t *)((uint8_t *)sys.apb + GPIO_BASE - APB_BASE + 0x100 * ip);
				uint32_t pin = p->PIN.D;
				uint32_t d = (pin ^ testInfo.gpio.pin[ip]) & testInfo.gpio.input[ip] & testInfo.gpio.mask[ip];
				if (d) {
					std::cout << "Found row pin(s) on port " << (char)('A' + ip)
							<< " 0x" << std::hex << std::setfill('0') << std::setw(8) << d << std::endl;
					testInfo.gpio.row[ip]  |= d;
					testInfo.gpio.mask[ip] &= ~d;
					found = true;
				}
			}
		}

		int result = mb.exec();
		mb.close();
		if (result != 0 || !found) {
			testInfo.testKeys = false;
			if (result == 0)
				NMQMessageBox::critical(ui.top, TR("错误"), TR("没有找到键盘矩阵 row GPIO"));
			return;
		}
	} else if (id == i++) {
		ui.updateStatus(TR("恢复键盘..."));
		int err = system("qcop 'QPE/System' 'setKeyboard(QString)' 'TTY:/dev/input/event0'");
		if (!WIFEXITED(err) || WEXITSTATUS(err) != 0) {
			NMQMessageBox::warning(ui.top, TR("注意"), TR("恢复键盘错误: %1 (%2)").arg(WEXITSTATUS(err)).arg(err));
			return;
		}
		if (testInfo.testKeys) {
			uint32_t cols = 0, rows = 0;
			for (int ip = 0; ip < 4; ip++) {
				cols += __builtin_popcountl(testInfo.gpio.column[ip]);
				rows += __builtin_popcountl(testInfo.gpio.row[ip]);
			}
			NMQMessageBox::information(ui.top, TR("键盘测试"), TR("找到了 %1 个 column GPIO\n以及 %2 个 row GPIO").arg(cols).arg(rows));
		}
	} else if (id == i++) {
		ui.updateStatus(TR("保存测试数据..."));
		std::string fname("data.txt");
		std::ofstream fs((testInfo.dir + "/" + fname).c_str());
		if (!fs.is_open()) {
			NMQMessageBox::critical(ui.top, TR("错误"), TR("无法创建文件:\n%1\n位于:\n%2").arg(fname.c_str()).arg(testInfo.dir.c_str()));
			return;
		}
		for (int ip = 0; ip < 4; ip++) {
			fs << "Port " << (char)('A' + ip) << ":" << std::endl;
			fs << "known   = 0x" << std::hex << std::setfill('0') << std::setw(8) << testInfo.gpio.known[ip]   << std::endl;
			fs << "mask    = 0x" << std::hex << std::setfill('0') << std::setw(8) << testInfo.gpio.mask[ip]    << std::endl;
			fs << "ignored = 0x" << std::hex << std::setfill('0') << std::setw(8) << testInfo.gpio.ignored[ip] << std::endl;
			fs << "row     = 0x" << std::hex << std::setfill('0') << std::setw(8) << testInfo.gpio.row[ip]     << std::endl;
			fs << "column  = 0x" << std::hex << std::setfill('0') << std::setw(8) << testInfo.gpio.column[ip]  << std::endl;
			fs << "input   = 0x" << std::hex << std::setfill('0') << std::setw(8) << testInfo.gpio.input[ip]   << std::endl;
			fs << "output  = 0x" << std::hex << std::setfill('0') << std::setw(8) << testInfo.gpio.output[ip]  << std::endl;
			fs << "pin     = 0x" << std::hex << std::setfill('0') << std::setw(8) << testInfo.gpio.pin[ip]     << std::endl;
			fs << std::endl;
		}
		if (!fs) {
			NMQMessageBox::critical(ui.top, TR("错误"), TR("写入文件失败:\n%1\n位于:\n%2").arg(fname.c_str()).arg(testInfo.dir.c_str()));
			return;
		}
		fs.close();
		sync();
	} else if (id == i++) {
		ui.updateStatus(TR("采集系统数据..."));

		system((std::string("hostname 2>&1 > ") + testInfo.dir + "/hostname").c_str());
		system((std::string("date 2>&1 > ") + testInfo.dir + "/date").c_str());

		system((std::string("mkdir -p ") + testInfo.dir + "/sys").c_str());
		system((std::string("dmesg 2>&1 > ") + testInfo.dir + "/sys/dmesg").c_str());
		system((std::string("df -ha 2>&1 > ") + testInfo.dir + "/sys/df").c_str());
		system((std::string("cat /proc/cpuinfo 2>&1 > ") + testInfo.dir + "/sys/cpuinfo").c_str());
		system((std::string("cat /proc/cmdline 2>&1 > ") + testInfo.dir + "/sys/cmdline").c_str());
		system((std::string("cat /proc/meminfo 2>&1 > ") + testInfo.dir + "/sys/meminfo").c_str());
		system((std::string("cat /proc/mtd 2>&1 > ") + testInfo.dir + "/sys/mtd").c_str());
		system((std::string("lsmod 2>&1 > ") + testInfo.dir + "/sys/lsmod").c_str());
		system((std::string("find /sys/class/ubi -type f -exec awk '{print FILENAME \":\\n\" $0}' {} \\; 2>&1 > ")
					+ testInfo.dir + "/sys/ubi").c_str());
		system((std::string("find /sys/class/graphics -type f -exec awk '{print FILENAME \":\\n\" $0}' {} \\; 2>&1 > ")
					+ testInfo.dir + "/sys/graphics").c_str());

		system((std::string("mkdir -p ") + testInfo.dir + "/tmp").c_str());
		system((std::string("cp -r /tmp/.test_data/* ") + testInfo.dir + "/tmp").c_str());
	} else if (id == i++) {
		ui.updateStatus(TR("压缩测试数据..."));
		int err = system((std::string("cd ") + testInfo.dir + "/..; tar jcf "
					+ basename(testInfo.dir.c_str()) + ".tar.bz2 " + basename(testInfo.dir.c_str())).c_str());
		if (!WIFEXITED(err) || WEXITSTATUS(err) != 0) {
			return;
		} else {
			err = system((std::string("rm -r ") + testInfo.dir).c_str());
			if (!WIFEXITED(err) || WEXITSTATUS(err) != 0)
				NMQMessageBox::warning(ui.top, TR("删除错误"), TR("请之后手动删除临时文件夹:\n%1").arg(testInfo.dir.c_str()));
			testInfo.dir += ".tar.bz2";
		}
#if 0
	} else if (id == i++) {
		ui.updateStatus(TR("debug"));
		system("qcop 'QPE/System' 'setKeyboard(QString)' 'TTY:/dev/input/event0'");
		testInfo.timer = -1;
#endif
	} else if (id == i++) {
		ui.updateStatus(TR("测试完成!"));
		testInfo.finished = 1;
		sync();
		NMQMessageBox::information(ui.top, TR("测试完成"), TR("测试数据保存在:\n%1").arg(testInfo.dir.c_str()));
		return;
	}
	testInfo.success++;
	std::cout << "Test " << std::dec << id << " success" << std::endl;
}

Test::Test(QObject *parent) : QObject(parent)
{
	init();
	if (failed())
		return;

	_tGPIO = startTimer(250);
	_tTest = startTimer(100);
}

Test::~Test()
{
	uninit();
}

void Test::timerEvent(QTimerEvent *e)
{
	if (e->timerId() == _tGPIO) {
		updateGPIO();
	} else if (e->timerId() == _tTest) {
		killTimer(_tTest);
		runTest();
	}
}

void Test::init()
{
	sys = (sys_t){0};
	_failed = false;

        // Open /dev/mem
        int mem = open("/dev/mem", O_RDWR | O_SYNC);
        if (mem == -1) {
		NMQMessageBox::critical(ui.top, tr("错误"), tr("无法访问 /dev/mem"));
		_failed = true;
		return;
	}
	sys.mem = mem;

	// APB block
        void *apb = mmap(0, APB_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem, APB_BASE);
        if (apb == (void *)-1) {
		NMQMessageBox::critical(ui.top, tr("错误"), tr("无法映射 APB 数据"));
		_failed = true;
		return;
	}
	sys.apb = apb;

	// AHB block
        void *ahb = mmap(0, AHB_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem, AHB_BASE);
        if (ahb == (void *)-1) {
		NMQMessageBox::critical(ui.top, tr("错误"), tr("无法映射 AHB 数据"));
		_failed = true;
		return;
	}
	sys.ahb = ahb;

	for (int i = 0; i < 4; i++)
		testInfo.gpio.mask[i] = 0xffffffff;

	ui.progress->setTotalSteps(12);
	ui.progress->setProgress(0);
	_test = 0;
}

void Test::uninit()
{
	if (sys.ahb)
		munmap(sys.ahb, 0x01000000);
	sys.ahb = 0;
	if (sys.apb)
		munmap(sys.apb, 0x01000000);
	sys.apb = 0;
	if (sys.mem)
		close(sys.mem);
	sys.mem = 0;
}

void Test::runTest()
{
	ui.updateStatus(TR("正在运行测试 %1...").arg(_test));
	std::cout <<  "Running test " << std::dec << _test << "..." << std::endl;
	test(_test++);
	if (!testInfo.finished) {
		ui.progress->setTotalSteps(testInfo.total + 2);
		ui.progress->setProgress(_test);
		if (testInfo.timer >= 0)
			_tTest = startTimer(testInfo.timer);
	} else {
		ui.progress->setTotalSteps(testInfo.total);
		ui.progress->setProgress(testInfo.total);
		ui.updateStatus(TR("测试完成!"));
		NMQMessageBox::information(ui.top, TR("测试完成"), TR("%1 个成功, %1 个失败")
									.arg(testInfo.success)
									.arg(testInfo.total - testInfo.success));
		qApp->quit();
	}
}

void Test::updateGPIO()
{
	for (int ip = 0; ip < 4; ip++) {
		gpio_t *p = (gpio_t *)((uint8_t *)sys.apb + GPIO_BASE - APB_BASE + 0x100 * ip);
		uint32_t pin = p->PIN.D, dat = p->DAT.D, fun = p->FUN.D, sel = p->SEL.D, dir = p->DIR.D;
		for (int i = 0; i < 4; i++) {
			QString s;
			for (int j = 0; j < 8; j++) {
				s += charGPIO(ip*32 + (31 - i*8) - j, !!(fun & 0x80000000), !!(sel & 0x80000000),
						!!(dir & 0x80000000), !!(dat & 0x80000000), !!(pin & 0x80000000));
				pin <<= 1;
				dat <<= 1;
				fun <<= 1;
				sel <<= 1;
				dir <<= 1;
			}
			ui.gpio[ip][i]->setText(s);
		}
	}
}

char Test::charGPIO(uint32_t idx, uint32_t fun, uint32_t sel, uint32_t dir, uint32_t dat, uint32_t pin)
{
	if (((testInfo.gpio.mask[idx / 32] >> (idx % 32)) & 1) == 0)
		return '-';
	char c = '?';
	switch (fun * 4 + sel * 2 + dir) {
	// GPIO, GPIO, IN
	case 0x00:	c = pin ? '1' : '0'; break;
	// GPIO, GPIO, OUT
	case 0x01:	c = pin ? 'H' : 'L'; break;
	// GPIO,  INT, LOW
	case 0x02:	c = 'v'; break;
	// GPIO,  INT, HIGH
	case 0x03:	c = '^'; break;
	//   AF,  AF0, -
	case 0x04:
	case 0x05:	c = 'f'; break;
	//   AF,  AF1, -
	case 0x06:
	case 0x07:	c = 'F'; break;
	}
	return c;
}
