/*
 * Copyright (C) 2004-2010 Geometer Plus <contact@geometerplus.com>
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

#include "ZLPaintContext.h"
#include "ZLStringUtil.h"

#include <algorithm>
#include <strings.h>

ZLPaintContext::ZLPaintContext() {
}

ZLPaintContext::~ZLPaintContext() {
}

std::string ZLPaintContext::pickFontFamily(const std::vector<std::string> &fonts) const
{
	if (!fonts.empty()) {
		if (fonts.size() > 1) {
			const std::vector<std::string> &available = fontFamilies();
			for (std::vector<std::string>::const_iterator it = fonts.begin(); it != fonts.end(); ++it) {
				const std::vector<std::string>::const_iterator found =
					std::lower_bound(available.begin(), available.end(), *it,
					ZLStringUtil::caseInsensitiveSort);
				if (found != available.end() && ZLStringUtil::caseInsensitiveEqual(*found, *it)) {
					return *found;
				}
			}
		}
		return fonts.front();
	}
	return std::string();
}

const std::vector<std::string> &ZLPaintContext::fontFamilies() const {
	if (myFamilies.empty()) {
		fillFamiliesList(myFamilies);
		std::sort(myFamilies.begin(), myFamilies.end(), ZLStringUtil::caseInsensitiveSort);
	}
	return myFamilies;
}

int ZLPaintContext::imageWidth(const ZLImageData &image, int width, int height, ScalingType type) const {
	const int origWidth = image.width();
	const int origHeight = image.height();
	if (origWidth == 0 || origHeight == 0) {
		return 0;
	}

	if ((origWidth <= width) && (origHeight <= height)) {
		if (type == SCALE_REDUCE_SIZE) {
			return origWidth;
		}
	} else {
		width = std::min(width, origWidth);
		height = std::min(height, origHeight);
	}
	if (origWidth * height < origHeight * width) {
		return (origWidth * height + origHeight / 2) / origHeight;
	}
	return width;
}

int ZLPaintContext::imageHeight(const ZLImageData &image, int width, int height, ScalingType type) const {
	const int origWidth = image.width();
	const int origHeight = image.height();
	if (origWidth == 0 || origHeight == 0) {
		return 0;
	}

	if ((origWidth <= width) && (origHeight <= height)) {
		if (type == SCALE_REDUCE_SIZE) {
			return origHeight;
		}
	} else {
		width = std::min(width, origWidth);
		height = std::min(height, origHeight);
	}
	if (origWidth * height > origHeight * width) {
		return (origHeight * width + origWidth / 2) / origWidth;
	}
	return height;
}
