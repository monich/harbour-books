/*
 * Copyright (C) 2015 Slava Monich <slava.monich@jolla.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "OCFContainerReader.h"
#include <string.h>

bool OCFContainerReader::readContainer(const ZLFile &file) {
	myOpfFile = std::string();
	myStateStack = std::stack<State>();
	myStateStack.push(STATE_CONTAINER);
	return readDocument(file);
}

void OCFContainerReader::startElementHandler(const char *tag, const char **attr) {
	switch (myStateStack.top()) {
	case STATE_CONTAINER:
		myStateStack.push(strcmp(tag, "container") ? STATE_IGNORE : STATE_ROOTFILES);
		break;
	case STATE_ROOTFILES:
		myStateStack.push(strcmp(tag, "rootfiles") ? STATE_IGNORE : STATE_ROOTFILE);
		break;
	case STATE_ROOTFILE:
		// An OCF Processor must consider the first rootfile element
		// within the rootfiles element to represent the Default
		// Rendition for the contained EPUB Publication. Reading
		// Systems are not required to use any Rendition except
		// the default one.
		if (myOpfFile.empty() && !strcmp(tag, "rootfile")) {
			const char *mediaType = attributeValue(attr, "media-type");
			if (mediaType && !strcmp(mediaType, "application/oebps-package+xml")) {
				const char *fullPath = attributeValue(attr, "full-path");
				if (fullPath) {
					myOpfFile = fullPath;
				}
			}
		}
		/* no break */
	default:
		myStateStack.push(STATE_IGNORE);
		break;
	}
}

void OCFContainerReader::endElementHandler(const char *tag) {
	myStateStack.pop();
}
