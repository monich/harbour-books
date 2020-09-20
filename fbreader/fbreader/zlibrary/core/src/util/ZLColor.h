/*
 * Copyright (C) 2004-2010 Geometer Plus <contact@geometerplus.com>
 * Copyright (C) 2015-2020 Slava Monich <slava.monich@jolla.com>
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

#ifndef __ZLCOLOR_H__
#define __ZLCOLOR_H__

struct ZLColor {
	unsigned char Alpha;
	unsigned char Red;
	unsigned char Green;
	unsigned char Blue;

	static const unsigned long ALPHA_MASK = 0xff000000;

	ZLColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
	ZLColor(unsigned char r, unsigned char g, unsigned char b);
	ZLColor(unsigned long argb = ALPHA_MASK);

	void setIntValue(unsigned long argb);
	unsigned long intValue();
	static unsigned long rgbValue(unsigned long rgb, unsigned char a = 0xff);

	bool equals(const ZLColor color) const;
	bool operator == (const ZLColor &color) const;
	bool operator != (const ZLColor &color) const;
};

inline ZLColor::ZLColor(unsigned char r, unsigned char g, unsigned char b) : Alpha(0xff), Red(r), Green(g), Blue(b) {}
inline ZLColor::ZLColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) : Alpha(a), Red(r), Green(g), Blue(b) {}
inline ZLColor::ZLColor(unsigned long argb) :  Alpha((unsigned char)(argb >> 24)), Red((unsigned char)(argb >> 16)), Green((unsigned char)(argb >> 8)), Blue((unsigned char)argb) {}
inline void ZLColor::setIntValue(unsigned long argb) { Alpha = (unsigned char)(argb >> 24); Red = (unsigned char)(argb >> 16); Green = (unsigned char)(argb >> 8); Blue = (unsigned char)argb; }
inline unsigned long ZLColor::intValue() { return (((unsigned long)Alpha) << 24) | (((unsigned long)Red) << 16) | (((unsigned long)Green) << 8) | (unsigned long)Blue; }
inline unsigned long ZLColor::rgbValue(unsigned long rgb, unsigned char a) { return (((unsigned long)a) << 24) | rgb; }
inline bool ZLColor::equals(const ZLColor color) const { return (Red == color.Red) && (Green == color.Green) && (Blue == color.Blue) && (Alpha == color.Alpha); }
inline bool ZLColor::operator == (const ZLColor &color) const { return equals(color); }
inline bool ZLColor::operator != (const ZLColor &color) const { return !equals(color); }

#endif /* __ZLCOLOR_H__ */
