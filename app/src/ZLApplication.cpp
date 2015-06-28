/*
 * Copyright (C) 2015 Jolla Ltd.
 * Contact: Slava Monich <slava.monich@jolla.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nemo Mobile nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "BooksDefs.h"

#include "HarbourDebug.h"

#include "ZLApplication.h"
#include "ZLApplicationWindow.h"
#include "ZLPaintContext.h"
#include "ZLViewWidget.h"
#include "ZLKeyBindings.h"
#include "ZLPopupData.h"
#include "ZLToolbar.h"
#include "ZLMenu.h"

// We are not currently using ZLApplication and ZLApplicationWindow but
// there are some references to it from the fbreader code which we must
// resolve.

ZLApplication *ZLApplication::ourInstance = NULL;

static const std::string ROTATION = "Rotation";
static const std::string ANGLE = "Angle";
static const std::string STATE = "State";
static const std::string KEYBOARD = "Keyboard";
static const std::string FULL_CONTROL = "FullControl";
static const std::string CONFIG = "Config";
static const std::string AUTO_SAVE = "AutoSave";
static const std::string TIMEOUT = "Timeout";

ZLApplication &ZLApplication::Instance()
{
    if (!ourInstance) new ZLApplication(BOOKS_APP_NAME);
    return *ourInstance;
}

void ZLApplication::deleteInstance()
{
    if (ourInstance) {
        delete ourInstance;
        ourInstance = NULL;
    }
}

ZLApplication::ZLApplication(const std::string &name) :
    ZLApplicationBase(name),
    RotationAngleOption(ZLCategoryKey::CONFIG, ROTATION, ANGLE, ZLView::DEGREES90),
    AngleStateOption(ZLCategoryKey::CONFIG, STATE, ANGLE, ZLView::DEGREES0),
    KeyboardControlOption(ZLCategoryKey::CONFIG, KEYBOARD, FULL_CONTROL, false),
    ConfigAutoSavingOption(ZLCategoryKey::CONFIG, CONFIG, AUTO_SAVE, true),
    ConfigAutoSaveTimeoutOption(ZLCategoryKey::CONFIG, CONFIG, TIMEOUT, 1, 6000, 30),
    KeyDelayOption(ZLCategoryKey::CONFIG, "Options", "KeyDelay", 0, 5000, 250)
{
    ourInstance = this;
}

ZLApplication::~ZLApplication()
{
}

void ZLApplication::initWindow()
{
}

shared_ptr<ZLKeyBindings> ZLApplication::keyBindings()
{
    return NULL;
}

bool ZLApplication::closeView()
{
    HDEBUG("NO");
    return false;
}

void ZLApplication::openFile(ZLFile const&)
{
}

bool ZLApplication::canDragFiles(std::vector<std::string, std::allocator<std::string> > const&) const
{
    HDEBUG("NO");
    return false;
}

void ZLApplication::dragFiles(std::vector<std::string, std::allocator<std::string> > const&)
{
    HDEBUG("WHAT?");
}

bool ZLApplication::isViewFinal() const
{
    HDEBUG("YES");
    return true;
}

void ZLApplication::refreshWindow()
{
    HDEBUG("OK");
}

void ZLApplication::setHyperlinkCursor(bool aHyperLink)
{
    HDEBUG(aHyperLink);
}

ZLApplicationWindow &ZLApplicationWindow::Instance()
{
    HDEBUG("THIS IS NOT SUPPOSED TO HAPPEN!");
    return *((ZLApplicationWindow*)NULL);
}

void ZLApplicationWindow::init()
{
}

void ZLApplicationWindow::refresh()
{
}
