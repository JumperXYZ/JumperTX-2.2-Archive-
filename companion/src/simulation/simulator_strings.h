#ifndef SIMULATOR_STRINGS_H
#define SIMULATOR_STRINGS_H

#include <QCoreApplication>

#define SIMU_STR_HLP_KEY_ENTER         QCoreApplication::translate("SimulatedUIWidget", "ENTER")
#define SIMU_STR_HLP_KEY_SHIFT         QCoreApplication::translate("SimulatedUIWidget", "S")
#define SIMU_STR_HLP_KEY_PGUP          QCoreApplication::translate("SimulatedUIWidget", "PG-UP")
#define SIMU_STR_HLP_KEY_PGDN          QCoreApplication::translate("SimulatedUIWidget", "PG-DN")
#define SIMU_STR_HLP_KEY_DEL           QCoreApplication::translate("SimulatedUIWidget", "DEL")
#define SIMU_STR_HLP_KEY_BKSP          QCoreApplication::translate("SimulatedUIWidget", "BKSP")
#define SIMU_STR_HLP_KEY_ESC           QCoreApplication::translate("SimulatedUIWidget", "ESC")
#define SIMU_STR_HLP_KEY_INS           QCoreApplication::translate("SimulatedUIWidget", "INS")
#define SIMU_STR_HLP_KEY_PLS           QCoreApplication::translate("SimulatedUIWidget", "<font size=+3>+</font>")
#define SIMU_STR_HLP_KEY_MIN           QCoreApplication::translate("SimulatedUIWidget", "<font size=+3>-</font>")

#define SIMU_STR_HLP_KEY_LFT           QCoreApplication::translate("SimulatedUIWidget", "<font size=+3>&larr;</font>")  // ←
#define SIMU_STR_HLP_KEY_RGT           QCoreApplication::translate("SimulatedUIWidget", "<font size=+3>&rarr;</font>")  // →
#define SIMU_STR_HLP_KEY_UP            QCoreApplication::translate("SimulatedUIWidget", "<font size=+3>&uarr;</font>")  // ↑
#define SIMU_STR_HLP_KEY_DN            QCoreApplication::translate("SimulatedUIWidget", "<font size=+3>&darr;</font>")  // ↓

#define SIMU_STR_HLP_ROTENC            QCoreApplication::translate("SimulatedUIWidget", "<font size=+3>&#x2686;</font>")  // ⚆
#define SIMU_STR_HLP_ROTENC_LFT        QCoreApplication::translate("SimulatedUIWidget", "<font size=+3>&#x21b6;</font>")  // ↶
#define SIMU_STR_HLP_ROTENC_RGT        QCoreApplication::translate("SimulatedUIWidget", "<font size=+3>&#x21b7;</font>")  // ↷
#define SIMU_STR_HLP_ROTENC_LR         QCoreApplication::translate("SimulatedUIWidget", "<font size=+3>&#x21c6;</font>")  // ⇆
#define SIMU_STR_HLP_ROTENC_DN         QCoreApplication::translate("SimulatedUIWidget", "<font size=+3>&#x21d3;</font>")  // ⇓

#define SIMU_STR_HLP_SCRL_UP           QCoreApplication::translate("SimulatedUIWidget", "<font size=+3>&#x21d1;</font>")  // ⇑
#define SIMU_STR_HLP_SCRL_DN           QCoreApplication::translate("SimulatedUIWidget", "<font size=+3>&#x21d3;</font>")  // ⇓
#define SIMU_STR_HLP_SCRL_UD           QCoreApplication::translate("SimulatedUIWidget", "<font size=+3>&#x21d5;</font>")  // ⇕

#define SIMU_STR_HLP_MOUSE             QCoreApplication::translate("SimulatedUIWidget", "<img src='qrc:/images/simulator/icons/svg/mouse.svg' width=20 height=18 />")
#define SIMU_STR_HLP_CLICK             QCoreApplication::translate("SimulatedUIWidget", "<img src='qrc:/images/simulator/icons/svg/arrow_click.svg' width=18 height=18 />")
#define SIMU_STR_HLP_MOUSE_MID         SIMU_STR_HLP_MOUSE % SIMU_STR_HLP_CLICK
#define SIMU_STR_HLP_MOUSE_UP          SIMU_STR_HLP_MOUSE % SIMU_STR_HLP_SCRL_UP
#define SIMU_STR_HLP_MOUSE_DN          SIMU_STR_HLP_MOUSE % SIMU_STR_HLP_SCRL_DN
#define SIMU_STR_HLP_MOUSE_SCRL        SIMU_STR_HLP_MOUSE % SIMU_STR_HLP_SCRL_UD

#define SIMU_STR_HLP_ACT_MENU          QCoreApplication::translate("SimulatedUIWidget", "<pre>[ MENU ]</pre>")
#define SIMU_STR_HLP_ACT_PAGE          QCoreApplication::translate("SimulatedUIWidget", "<pre>[ PAGE ]</pre>")
#define SIMU_STR_HLP_ACT_EXIT          QCoreApplication::translate("SimulatedUIWidget", "<pre>[ EXIT ]</pre>")
#define SIMU_STR_HLP_ACT_ENT           QCoreApplication::translate("SimulatedUIWidget", "<pre>[ ENT ]</pre>")
#define SIMU_STR_HLP_ACT_SHIFT         QCoreApplication::translate("SimulatedUIWidget", "<pre>[ SHIFT ]</pre>")
#define SIMU_STR_HLP_ACT_UP            QCoreApplication::translate("SimulatedUIWidget", "<pre>[ UP ]</pre>")
#define SIMU_STR_HLP_ACT_DN            QCoreApplication::translate("SimulatedUIWidget", "<pre>[ DN ]</pre>")
#define SIMU_STR_HLP_ACT_PLS           QCoreApplication::translate("SimulatedUIWidget", "<pre>[ <font size=+2>+</font> ]</pre>")
#define SIMU_STR_HLP_ACT_MIN           QCoreApplication::translate("SimulatedUIWidget", "<pre>[ <font size=+2>-</font> ]</pre>")
#define SIMU_STR_HLP_ACT_PGUP          QCoreApplication::translate("SimulatedUIWidget", "<pre>[ PgUp ]</pre>")
#define SIMU_STR_HLP_ACT_PGDN          QCoreApplication::translate("SimulatedUIWidget", "<pre>[ PgDn ]</pre>")
#define SIMU_STR_HLP_ACT_MDL           QCoreApplication::translate("SimulatedUIWidget", "<pre>[ MDL ]</pre>")
#define SIMU_STR_HLP_ACT_RTN           QCoreApplication::translate("SimulatedUIWidget", "<pre>[ RTN ]</pre>")
#define SIMU_STR_HLP_ACT_SYS           QCoreApplication::translate("SimulatedUIWidget", "<pre>[ SYS ]</pre>")
#define SIMU_STR_HLP_ACT_TELE          QCoreApplication::translate("SimulatedUIWidget", "<pre>[ TELE ]</pre>")
#define SIMU_STR_HLP_ACT_MENU_ICN      QCoreApplication::translate("SimulatedUIWidget", "<font size=+3>&#x2261;</font>")  // ≡


#define SIMU_STR_HLP_KEYS_EXIT         SIMU_STR_HLP_KEY_DEL % "|" % SIMU_STR_HLP_KEY_BKSP % "|" % SIMU_STR_HLP_KEY_ESC
#define SIMU_STR_HLP_KEYS_ACTIVATE     SIMU_STR_HLP_KEY_ENTER % "|" % SIMU_STR_HLP_MOUSE_MID
#define SIMU_STR_HLP_KEYS_GO_UP        SIMU_STR_HLP_KEY_UP % "|" % SIMU_STR_HLP_KEY_PGUP
#define SIMU_STR_HLP_KEYS_GO_DN        SIMU_STR_HLP_KEY_DN % "|" % SIMU_STR_HLP_KEY_PGDN
#define SIMU_STR_HLP_KEYS_GO_LFT       SIMU_STR_HLP_KEY_LFT % "|" % SIMU_STR_HLP_KEY_MIN % "|" % SIMU_STR_HLP_MOUSE_UP
#define SIMU_STR_HLP_KEYS_GO_RGT       SIMU_STR_HLP_KEY_RGT % "|" % SIMU_STR_HLP_KEY_PLS % "|" % SIMU_STR_HLP_MOUSE_DN

#define SIMU_STR_HLP_ACT_ROT_LFT       SIMU_STR_HLP_ROTENC % " " % SIMU_STR_HLP_ROTENC_LFT
#define SIMU_STR_HLP_ACT_ROT_RGT       SIMU_STR_HLP_ROTENC % " " % SIMU_STR_HLP_ROTENC_RGT
#define SIMU_STR_HLP_ACT_ROT_DN        SIMU_STR_HLP_ROTENC % " " % SIMU_STR_HLP_ROTENC_DN

#endif // SIMULATOR_STRINGS_H
