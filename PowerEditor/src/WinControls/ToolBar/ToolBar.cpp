//this file is part of notepad++
//Copyright (C)2003 Don HO ( donho@altern.org )
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

#include "precompiled_headers.h"

#include "MISC/PluginsManager/Notepad_plus_msgs.h"

#include "WinControls/shortcut/shortcut.h"
#include "WinControls/ImageListSet/ImageListSet.h"

#include "WinControls/ToolBar/Toolbar.h"

const int WS_TOOLBARSTYLE = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBSTYLE_TOOLTIPS |TBSTYLE_FLAT | CCS_TOP | BTNS_AUTOSIZE | CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER;

ToolBar::ToolBar():
	_pTBB(NULL),
	_toolBarIcons(NULL),
	_state(TB_STANDARD),
	_nrButtons(0),
	_nrDynButtons(0),
	_nrTotalButtons(0),
	_nrCurrentButtons(0),
	_pRebar(NULL),
	_toolIcons(NULL)
{
	memset(&_rbBand, 0, sizeof(REBARBANDINFO));
}

ToolBar::~ToolBar()
{
	ToolBar::destroy();
}

void ToolBar::initTheme(TiXmlDocument *toolIconsDocRoot)
{
	assert(toolIconsDocRoot);
	// JOCE: Check if we could use XPath style accessors with TinyXML.
    _toolIcons =  toolIconsDocRoot->FirstChild(TEXT("NotepadPlus"));
	if (_toolIcons)
	{
		_toolIcons = _toolIcons->FirstChild(TEXT("ToolBarIcons"));
		if (_toolIcons)
		{
			_toolIcons = _toolIcons->FirstChild(TEXT("Theme"));
			if (_toolIcons)
			{
				const TCHAR *themeDir = (_toolIcons->ToElement())->Attribute(TEXT("pathPrefix"));

				for (TiXmlNode *childNode = _toolIcons->FirstChildElement(TEXT("Icon"));
					 childNode ;
					 childNode = childNode->NextSibling(TEXT("Icon")))
				{
					int iIcon;
					const TCHAR *res = (childNode->ToElement())->Attribute(TEXT("id"), &iIcon);
					if (res)
					{
						TiXmlNode *grandChildNode = childNode->FirstChildElement(TEXT("normal"));
						if (grandChildNode)
						{
							TiXmlNode *valueNode = grandChildNode->FirstChild();
							if (valueNode)
							{
								generic_string locator = themeDir?themeDir:TEXT("");

								locator += valueNode->Value();
								_customIconVect.push_back(iconLocator(0, iIcon, locator));
							}
						}

						grandChildNode = childNode->FirstChildElement(TEXT("hover"));
						if (grandChildNode)
						{
							TiXmlNode *valueNode = grandChildNode->FirstChild();
							if (valueNode)
							{
								generic_string locator = themeDir?themeDir:TEXT("");

								locator += valueNode->Value();
								_customIconVect.push_back(iconLocator(1, iIcon, locator));
							}
						}

						grandChildNode = childNode->FirstChildElement(TEXT("disabled"));
						if (grandChildNode)
						{
							TiXmlNode *valueNode = grandChildNode->FirstChild();
							if (valueNode)
							{
								generic_string locator = themeDir?themeDir:TEXT("");

								locator += valueNode->Value();
								_customIconVect.push_back(iconLocator(2, iIcon, locator));
							}
						}
					}
				}
			}
		}
	}
}

bool ToolBar::init( HINSTANCE hInst, HWND hParent, toolBarStatusType type,
					ToolBarButtonUnit *buttonUnitArray, int arraySize)
{
	Window::init(hInst, hParent);
	_state = type;
	int iconSize = (_state == TB_LARGE?32:16);

	_toolBarIcons = new ToolBarIcons;
	_toolBarIcons->init(buttonUnitArray, arraySize);
	_toolBarIcons->create(_hInst, iconSize);

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_WIN95_CLASSES|ICC_COOL_CLASSES|ICC_BAR_CLASSES|ICC_USEREX_CLASSES;
	InitCommonControlsEx(&icex);

	//Create the list of buttons
	_nrButtons    = arraySize;
	_nrDynButtons = _vDynBtnReg.size();
	_nrTotalButtons = _nrButtons + (_nrDynButtons ? _nrDynButtons + 1 : 0);
	_pTBB = new TBBUTTON[_nrTotalButtons];	//add one for the extra separator

	int cmd = 0;
	int bmpIndex = -1;
	BYTE style;
	size_t i = 0;
	for (; i < _nrButtons ; i++)
	{
		cmd = buttonUnitArray[i]._cmdID;
		if (cmd != 0)
		{
			bmpIndex++;
			style = BTNS_BUTTON;
		}
		else
		{
			style = BTNS_SEP;
		}

		_pTBB[i].iBitmap = (cmd != 0?bmpIndex:0);
		_pTBB[i].idCommand = cmd;
		_pTBB[i].fsState = TBSTATE_ENABLED;
		_pTBB[i].fsStyle = style;
		_pTBB[i].dwData = 0;
		_pTBB[i].iString = 0;
	}

	if (_nrDynButtons > 0) {
		//add separator
		_pTBB[i].iBitmap = 0;
		_pTBB[i].idCommand = 0;
		_pTBB[i].fsState = TBSTATE_ENABLED;
		_pTBB[i].fsStyle = BTNS_SEP;
		_pTBB[i].dwData = 0;
		_pTBB[i].iString = 0;
		i++;
		//add plugin buttons
		for (size_t j = 0; j < _nrDynButtons ; j++, i++)
		{
			cmd = _vDynBtnReg[j].message;
			bmpIndex++;

			_pTBB[i].iBitmap = bmpIndex;
			_pTBB[i].idCommand = cmd;
			_pTBB[i].fsState = TBSTATE_ENABLED;
			_pTBB[i].fsStyle = BTNS_BUTTON;
			_pTBB[i].dwData = 0;
			_pTBB[i].iString = 0;
		}
	}

	reset(true);	//load icons etc

	return true;
}

void ToolBar::destroy()
{
	if (_pRebar) {
		_pRebar->removeBand(_rbBand.wID);
		_pRebar = NULL;
	}

	if (_pTBB)
	{
		delete [] _pTBB;
		_pTBB = NULL;
	}

	if (_toolBarIcons)
	{
		delete _toolBarIcons;
		_toolBarIcons = NULL;
	}

	Window::destroy();
};

int ToolBar::getWidth() const
{
	RECT btnRect;
	int totalWidth = 0;
	for(size_t i = 0; i < _nrCurrentButtons; i++) {
		::SendMessage(_hSelf, TB_GETITEMRECT, i, (LPARAM)&btnRect);
		totalWidth += btnRect.right - btnRect.left;
	}
	return totalWidth;
}

int ToolBar::getHeight() const
{
	DWORD size = (DWORD)SendMessage(_hSelf, TB_GETBUTTONSIZE, 0, 0);
    DWORD padding = (DWORD)SendMessage(_hSelf, TB_GETPADDING, 0,0);
	int totalHeight = HIWORD(size) + HIWORD(padding);

	return totalHeight;
}

void ToolBar::reduce()
{
	if (_state == TB_SMALL)
		return;

	_toolBarIcons->resizeIcon(16);
	bool recreate = (_state == TB_STANDARD);
	setState(TB_SMALL);
	reset(recreate);	//recreate toolbar if std icons were used
	Window::redraw();
}

void ToolBar::enlarge()
{
	if (_state == TB_LARGE)
		return;

	_toolBarIcons->resizeIcon(32);
	bool recreate = (_state == TB_STANDARD);
	setState(TB_LARGE);
	reset(recreate);	//recreate toolbar if std icons were used
	Window::redraw();
}

void ToolBar::setToUglyIcons()
{
	if (_state == TB_STANDARD)
		return;
	bool recreate = true;
	setState(TB_STANDARD);
	reset(recreate);	//must recreate toolbar if setting to internal bitmaps
	Window::redraw();
}

bool ToolBar::getCheckState(int ID2Check) const
{
	return bool(::SendMessage(_hSelf, TB_GETSTATE, (WPARAM)ID2Check, 0) & TBSTATE_CHECKED);
}

void ToolBar::setCheck(int ID2Check, bool willBeChecked) const
{
	::SendMessage(_hSelf, TB_CHECKBUTTON, (WPARAM)ID2Check, (LPARAM)MAKELONG(willBeChecked, 0));
}

bool ToolBar::changeIcons()
{
	if (!_toolIcons)
		return false;
	for (int i = 0 ; i < int(_customIconVect.size()) ; i++)
		changeIcons(_customIconVect[i].listIndex, _customIconVect[i].iconIndex, (_customIconVect[i].iconLocation).c_str());
	return true;
}

bool ToolBar::changeIcons(int whichLst, int iconIndex, const TCHAR *iconLocation)
{
	return _toolBarIcons->replaceIcon(whichLst, iconIndex, iconLocation);
}

void ToolBar::reset(bool create)
{

	if(create && _hSelf) {
		//Store current button state information
		TBBUTTON tempBtn;
		for(size_t i = 0; i < _nrCurrentButtons; i++) {
			::SendMessage(_hSelf, TB_GETBUTTON, (WPARAM)i, (LPARAM)&tempBtn);
			_pTBB[i].fsState = tempBtn.fsState;
		}
		::DestroyWindow(_hSelf);
		_hSelf = NULL;
	}

	if(!_hSelf) {
		_hSelf = ::CreateWindowEx(
					WS_EX_PALETTEWINDOW,
					TOOLBARCLASSNAME,
					TEXT(""),
					WS_TOOLBARSTYLE,
					0, 0,
					0, 0,
					_hParent,
					NULL,
					_hInst,
					0);
		// Send the TB_BUTTONSTRUCTSIZE message, which is required for
		// backward compatibility.
		::SendMessage(_hSelf, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
		::SendMessage(_hSelf, TB_SETEXTENDEDSTYLE, 0, (LPARAM)TBSTYLE_EX_HIDECLIPPEDBUTTONS);
	}

	if (!_hSelf)
	{
		throw std::runtime_error("ToolBar::reset : CreateWindowEx() function return null");
	}

	if (_state != TB_STANDARD)
	{
		//If non standard icons, use custom imagelists
		setDefaultImageList();
		setHotImageList();
		setDisableImageList();
	}
	else
	{
		//Else set the internal imagelist with standard bitmaps
		TBADDBITMAP addbmp = {_hInst, 0};
		TBADDBITMAP addbmpdyn = {0, 0};
		for (size_t i = 0 ; i < _nrButtons ; i++)
		{
			addbmp.nID = _toolBarIcons->getStdIconAt(i);
			::SendMessage(_hSelf, TB_ADDBITMAP, 1, (LPARAM)&addbmp);
		}
		if (_nrDynButtons > 0) {
			for (size_t j = 0; j < _nrDynButtons; j++)
			{
				addbmpdyn.nID = (UINT_PTR)_vDynBtnReg.at(j).hBmp;
				::SendMessage(_hSelf, TB_ADDBITMAP, 1, (LPARAM)&addbmpdyn);
			}
		}
	}

	if (create) {	//if the toolbar has been recreated, readd the buttons
		size_t nrBtnToAdd = (_state == TB_STANDARD?_nrTotalButtons:_nrButtons);
		_nrCurrentButtons = nrBtnToAdd;
		WORD btnSize = (_state == TB_LARGE?32:16);
		::SendMessage(_hSelf, TB_SETBUTTONSIZE , (WPARAM)0, (LPARAM)MAKELONG (btnSize, btnSize));
		::SendMessage(_hSelf, TB_ADDBUTTONS, (WPARAM)nrBtnToAdd, (LPARAM)_pTBB);
	}
	::SendMessage(_hSelf, TB_AUTOSIZE, 0, 0);

	if (_pRebar) {
		_rbBand.hwndChild	= getHSelf();
		_rbBand.cxMinChild	= 0;
		_rbBand.cyIntegral	= 1;
		_rbBand.cyMinChild	= _rbBand.cyMaxChild	= getHeight();
		_rbBand.cxIdeal		= getWidth();

		_pRebar->reNew(REBAR_BAR_TOOLBAR, &_rbBand);
	}
}

void ToolBar::registerDynBtn(UINT messageID, toolbarIcons* tIcon)
{
	// Note: Register of buttons only possible before init!
	if ((_hSelf == NULL) && (messageID != 0) && (tIcon->hToolbarBmp != NULL))
	{
		tDynamicList		dynList;
		dynList.message		= messageID;
		dynList.hBmp		= tIcon->hToolbarBmp;
		dynList.hIcon		= tIcon->hToolbarIcon;
		_vDynBtnReg.push_back(dynList);
	}
}

void ToolBar::doPopop(POINT chevPoint) {
	//first find hidden buttons
	int width = Window::getWidth();

	size_t start = 0;
	RECT btnRect = {0,0,0,0};
	while(start < _nrCurrentButtons) {
		::SendMessage(_hSelf, TB_GETITEMRECT, start, (LPARAM)&btnRect);
		if(btnRect.right > width)
			break;
		start++;
	}

	if (start < _nrCurrentButtons) {	//some buttons are hidden
		HMENU menu = ::CreatePopupMenu();
		int cmd;
		generic_string text;
		while (start < _nrCurrentButtons) {
			cmd = _pTBB[start].idCommand;
			getNameStrFromCmd(cmd, text);
			if (_pTBB[start].idCommand != 0) {
				if (::SendMessage(_hSelf, TB_ISBUTTONENABLED, cmd, 0) != 0)
					AppendMenu(menu, MF_ENABLED, cmd, text.c_str());
				else
					AppendMenu(menu, MF_DISABLED|MF_GRAYED, cmd, text.c_str());
			} else
				AppendMenu(menu, MF_SEPARATOR, 0, TEXT(""));
			start++;
		}
		TrackPopupMenu(menu, 0, chevPoint.x, chevPoint.y, 0, _hSelf, NULL);
	}
}

void ToolBar::addToRebar(ReBar * rebar)
{
	if (_pRebar)
		return;
	_pRebar = rebar;
	ZeroMemory(&_rbBand, REBARBAND_SIZE);
	_rbBand.cbSize  = REBARBAND_SIZE;

	_rbBand.fMask   = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE |
					  RBBIM_SIZE | RBBIM_IDEALSIZE | RBBIM_ID;

	_rbBand.fStyle		= RBBS_VARIABLEHEIGHT | RBBS_USECHEVRON;
	_rbBand.hwndChild	= getHSelf();
	_rbBand.wID			= REBAR_BAR_TOOLBAR;	//ID REBAR_BAR_TOOLBAR for toolbar
	_rbBand.cxMinChild	= 0;
	_rbBand.cyIntegral	= 1;
	_rbBand.cyMinChild	= _rbBand.cyMaxChild	= getHeight();
	_rbBand.cxIdeal		= _rbBand.cx			= getWidth();

	_pRebar->addBand(&_rbBand, true);

	_rbBand.fMask   = RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_IDEALSIZE | RBBIM_SIZE;
}

void ToolBar::setDefaultImageList() {
	::SendMessage(_hSelf, TB_SETIMAGELIST , (WPARAM)0, (LPARAM)_toolBarIcons->getDefaultLst());
}

void ToolBar::setHotImageList() {
	::SendMessage(_hSelf, TB_SETHOTIMAGELIST , (WPARAM)0, (LPARAM)_toolBarIcons->getHotLst());
}

void ToolBar::setDisableImageList() {
	::SendMessage(_hSelf, TB_SETDISABLEDIMAGELIST, (WPARAM)0, (LPARAM)_toolBarIcons->getDisableLst());
};


ReBar::~ReBar()
{
	ReBar::destroy();
}

void ReBar::init(HINSTANCE hInst, HWND hParent)
{
	Window::init(hInst, hParent);

	_hSelf = CreateWindowEx(WS_EX_TOOLWINDOW,
							REBARCLASSNAME,
							NULL,
							WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|RBS_VARHEIGHT|
							RBS_BANDBORDERS | CCS_NODIVIDER | CCS_NOPARENTALIGN,
							0,0,0,0, _hParent, NULL, _hInst, NULL);

	REBARINFO rbi;
	ZeroMemory(&rbi, sizeof(REBARINFO));
	rbi.cbSize = sizeof(REBARINFO);
	rbi.fMask  = 0;
	rbi.himl   = (HIMAGELIST)NULL;
	::SendMessage(_hSelf, RB_SETBARINFO, 0, (LPARAM)&rbi);
}

bool ReBar::addBand(REBARBANDINFO * rBand, bool useID)
{
	if (rBand->fMask & RBBIM_STYLE)
		rBand->fStyle |= RBBS_GRIPPERALWAYS;
	else
		rBand->fStyle = RBBS_GRIPPERALWAYS;
	rBand->fMask |= RBBIM_ID | RBBIM_STYLE;
	if (useID) {
		if (isIDTaken(rBand->wID))
			return false;

	} else {
		rBand->wID = getNewID();
	}
	::SendMessage(_hSelf, RB_INSERTBAND, (WPARAM)-1, (LPARAM)rBand);	//add to end of list
	return true;
}

void ReBar::reNew(int id, REBARBANDINFO * rBand)
{
	int index = (int)SendMessage(_hSelf, RB_IDTOINDEX, (WPARAM)id, 0);
	::SendMessage(_hSelf, RB_SETBANDINFO, (WPARAM)index, (LPARAM)rBand);
};

void ReBar::removeBand(int id)
{
	int index = (int)SendMessage(_hSelf, RB_IDTOINDEX, (WPARAM)id, 0);
	if (id >= REBAR_BAR_EXTERNAL)
		releaseID(id);
	::SendMessage(_hSelf, RB_DELETEBAND, (WPARAM)index, (LPARAM)0);
}

void ReBar::setIDVisible(int id, bool show)
{
	int index = (int)SendMessage(_hSelf, RB_IDTOINDEX, (WPARAM)id, 0);
	if (index == -1 )
		return;	//error

	REBARBANDINFO rbBand;
	ZeroMemory(&rbBand, REBARBAND_SIZE);
	rbBand.cbSize  = REBARBAND_SIZE;

	rbBand.fMask = RBBIM_STYLE;
	::SendMessage(_hSelf, RB_GETBANDINFO, (WPARAM)index, (LPARAM)&rbBand);
	if (show)
		rbBand.fStyle &= (RBBS_HIDDEN ^ -1);
	else
		rbBand.fStyle |= RBBS_HIDDEN;
	::SendMessage(_hSelf, RB_SETBANDINFO, (WPARAM)index, (LPARAM)&rbBand);
}

bool ReBar::getIDVisible(int id)
{
	int index = (int)SendMessage(_hSelf, RB_IDTOINDEX, (WPARAM)id, 0);
	if (index == -1 )
		return false;	//error
	REBARBANDINFO rbBand;
	ZeroMemory(&rbBand, REBARBAND_SIZE);
	rbBand.cbSize  = REBARBAND_SIZE;

	rbBand.fMask = RBBIM_STYLE;
	::SendMessage(_hSelf, RB_GETBANDINFO, (WPARAM)index, (LPARAM)&rbBand);
	return ((rbBand.fStyle & RBBS_HIDDEN) == 0);
}

int ReBar::getNewID()
{
	int idToUse = REBAR_BAR_EXTERNAL;
	int curVal = 0;
	size_t size = usedIDs.size();
	for(size_t i = 0; i < size; i++)
	{
		curVal = usedIDs.at(i);
		if (curVal < idToUse)
		{
			continue;
		}
		else if (curVal == idToUse)
		{
			idToUse++;
		}
		else
		{
			break;		//found gap
		}
	}

	usedIDs.push_back(idToUse);
	return idToUse;
}

void ReBar::releaseID(int id)
{
	size_t size = usedIDs.size();
	for(size_t i = 0; i < size; i++)
	{
		if (usedIDs.at(i) == id)
		{
			usedIDs.erase(usedIDs.begin()+i);
			break;
		}
	}
}

bool ReBar::isIDTaken(int id)
{
	size_t size = usedIDs.size();
	for(size_t i = 0; i < size; i++)
	{
		if (usedIDs.at(i) == id)
		{
			return true;
		}
	}
	return false;
}

