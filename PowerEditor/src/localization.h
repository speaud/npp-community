//this file is part of notepad++
//Copyright (C)2010 Don HO <don.h@free.fr>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef LOCALIZATION_H
#define LOCALIZATION_H

class ShortcutMapper;
class TiXmlNodeA;
class TiXmlDocumentA;
class UserDefineDialog;
class FindReplaceDlg;
class PreferenceDlg;

class NativeLangSpeaker {
public:
    NativeLangSpeaker();
    void init(TiXmlDocumentA* nativeLangDocRootA, bool loadIfEnglish = false);
	void changeConfigLang(HWND hDlg);
	void changeLangTabContextMenu(HMENU hCM);
	TiXmlNodeA * searchDlgNode(TiXmlNodeA *node, const char *dlgTagName);
	bool changeDlgLang(HWND hDlg, const char *dlgTagName, char *title = NULL, int titleBufLen = 0);
	void changeLangTabDrapContextMenu(HMENU hCM);
	void changeMenuLang(HMENU menuHandle, generic_string& pluginsTrans, generic_string& windowTrans);
	void changeShortcutLang();
	void changeShortcutmapperLang(ShortcutMapper* sm);
	void changeStyleCtrlsLang(HWND hDlg, int *idArray, const char **translatedText);
	// JOCE: The changes should all be the dialogs themselves
    void changeUserDefineLang(UserDefineDialog* userDefineDlg);
    void changeFindReplaceDlgLang(FindReplaceDlg* findReplaceDlg);
    void changePrefereceDlgLang(PreferenceDlg* preference);
    bool isRTL() const {
        return _isRTL;
    };

    const char * getFileName() const {
        return _fileName.c_str();
    };

    const TiXmlNodeA * getNativeLangA() const {
        return _nativeLangA;
    };

    int getLangEncoding() const {
        return _nativeLangEncoding;
    };
private:
	TiXmlNodeA *_nativeLangA;
	int _nativeLangEncoding;
    bool _isRTL;
    std::basic_string<char> _fileName;
};

#endif // LOCALIZATION_H
