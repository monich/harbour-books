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

#ifndef __OCFCONTANERREADER_H__
#define __OCFCONTANERREADER_H__

#include <ZLXMLReader.h>

#include <stack>

class OCFContainerReader : public ZLXMLReader {
public:
	OCFContainerReader() {}

	bool readContainer(const ZLFile &file);
	const std::string &opfFile() const;

private:
	void startElementHandler(const char *tag, const char **attr);
	void endElementHandler(const char *tag);

private:
	enum State {
		STATE_CONTAINER,
		STATE_ROOTFILES,
		STATE_ROOTFILE,
		STATE_IGNORE
	};

	std::stack<State> myStateStack;
	std::string myOpfFile;
};

inline const std::string &OCFContainerReader::opfFile() const { return myOpfFile; }

#endif /* __OCFCONTANERREADER_H__ */
