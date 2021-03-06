// Scintilla source code edit control
/** @file CharClassify.cxx
 ** Character classifications used by Document and RESearch.
 **/
// Copyright 2006 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

// NPPSTART Joce 06/09/09 Scintilla_precomp_headers
#include "precompiled_headers.h"
//#include <stdlib.h>
//#include <ctype.h>
// NPPEND

#include "CharClassify.h"

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

// NPPSTART Joce 06/16/09 Scintilla_clean_precomp
// All warnings are now enabled.
//// Shut up annoying Visual C++ warnings:
//#ifdef _MSC_VER
//#pragma warning(disable: 4514)
//#endif
// NPPEND

CharClassify::CharClassify() {
	SetDefaultCharClasses(true);
}

void CharClassify::SetDefaultCharClasses(bool includeWordClass) {
	// Initialize all char classes to default values
	for (int ch = 0; ch < 256; ch++) {
		if (ch == '\r' || ch == '\n')
			charClass[ch] = ccNewLine;
		else if (ch < 0x20 || ch == ' ')
			charClass[ch] = ccSpace;
		else if (includeWordClass && (ch >= 0x80 || isalnum(ch) || ch == '_'))
			charClass[ch] = ccWord;
		else
			charClass[ch] = ccPunctuation;
	}
}

void CharClassify::SetCharClasses(const unsigned char *chars, cc newCharClass) {
	// Apply the newCharClass to the specifed chars
	if (chars) {
		while (*chars) {
			charClass[*chars] = static_cast<unsigned char>(newCharClass);
			chars++;
		}
	}
}
