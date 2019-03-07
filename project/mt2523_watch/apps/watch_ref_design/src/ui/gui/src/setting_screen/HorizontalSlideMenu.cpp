/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#include <gui/setting_screen/HorizontalSlideMenu.hpp>
#include <BitmapDatabase.hpp>
#include <touchgfx/EasingEquations.hpp>
#include <gui/common/CommonUI.hpp>

HorizontalSlideMenu::HorizontalSlideMenu() :
    size(0),
    animationDuration(10),
    animationEndedCallback(this, &HorizontalSlideMenu::animationEndedHandler)
{
}

void HorizontalSlideMenu::setup(int numberOfElements, int selectedElementIndex)
{
    assert(selectedElementIndex < numberOfElements);
    assert(numberOfElements <= MAX_SIZE);

    size = numberOfElements;
    imagesXPositionDelta = CommonUI::SLIDE_MENU_IMGAE_X_DELTA;

    smallImageWidth = CommonUI::SLIDE_MENU_SMALL_IMAGE_WIDTH;
    smallImageHeight = CommonUI::SLIDE_MENU_SMALL_IMAGE_HEIGHT;
    largeImageWidth = CommonUI::SLIDE_MENU_LARGE_IMAGE_WIDTH;
    largeImageHeight = CommonUI::SLIDE_MENU_LARGE_IMAGE_HEIGHT;
    largeImageX = (getWidth() - largeImageWidth) / 2;
    largeImageY = CommonUI::SLIDE_MENU_LARGE_IMAGE_Y;
    smallImageY = CommonUI::SLIDE_MENU_SMALL_IMAGE_Y;

    currentSelected = selectedElementIndex;

    images[currentSelected].setWidth(largeImageWidth);
    images[currentSelected].setHeight(largeImageHeight);
    images[currentSelected].setXY(largeImageX, largeImageY);
    images[currentSelected].setAnimationEndedCallback(animationEndedCallback);
    add(images[currentSelected]);

    for (int i = currentSelected - 1; i >= 0; i--) {
        images[i].setWidth(smallImageWidth);
        images[i].setHeight(smallImageHeight);

        images[i].setXY(images[i + 1].getX() - smallImageWidth - imagesXPositionDelta, smallImageY);

        images[i].setAnimationEndedCallback(animationEndedCallback);
        add(images[i]);
    }

    for (int i = currentSelected + 1; i < size; i++) {
        images[i].setWidth(smallImageWidth);
        images[i].setHeight(smallImageHeight);

        images[i].setXY(images[i - 1].getX() + images[i - 1].getWidth() + imagesXPositionDelta, smallImageY);

        images[i].setAnimationEndedCallback(animationEndedCallback);
        add(images[i]);
    }
}

HorizontalSlideMenu::~HorizontalSlideMenu()
{
}

void HorizontalSlideMenu::handleGestureEvent(const GestureEvent &evt)
{
    if (evt.getType() == evt.SWIPE_HORIZONTAL) {
        int selectedElement = getSelectedElementIndex();

        if (evt.getVelocity() < 0 && selectedElement < getSize() - 1) {
            animateLeft();
        } else if (evt.getVelocity() > 0 && selectedElement > 0) {
            animateRight();
        }
    }
}

void HorizontalSlideMenu::setBitmapsForElement(int elementIndex, BitmapId smallBmp, BitmapId largeBmp)
{
    // Make sure that the dimensions of the bitmap stays the same
    int oldWidth = images[elementIndex].getWidth();
    int oldHeight = images[elementIndex].getHeight();

    images[elementIndex].setBitmaps(Bitmap(smallBmp), Bitmap(largeBmp));

    images[elementIndex].setWidth(oldWidth);
    images[elementIndex].setHeight(oldHeight);
}

void HorizontalSlideMenu::animateLeft()
{
    if (animating() || currentSelected == size - 1) {
        return;
    }

    for (int i = 0; i < size; i++) {
        // If image is not visible and will not be after animation just move it
        if (images[i].getX() + images[i].getWidth() < 0) {
            images[i].moveTo(images[i].getX() - images[i].getWidth() - imagesXPositionDelta, images[i].getY());
        }
        // If image is not visible and will not be after animation just move it
        else if (images[i].getX() - images[i].getWidth() - imagesXPositionDelta > getWidth()) {
            images[i].moveTo(images[i].getX() - images[i].getWidth() - imagesXPositionDelta, images[i].getY());
        } else if (i == currentSelected + 1) {
            images[i].startZoomAndMoveAnimation(images[i].getX() - largeImageWidth - imagesXPositionDelta, largeImageY, largeImageWidth, largeImageHeight, animationDuration,
                                                ZoomAnimationImage::FIXED_LEFT_AND_TOP, EasingEquations::linearEaseNone, EasingEquations::linearEaseNone, EasingEquations::linearEaseNone, EasingEquations::linearEaseNone);
        } else if (i == currentSelected) {
            images[i].startZoomAndMoveAnimation(images[i].getX() - smallImageWidth - imagesXPositionDelta, smallImageY, smallImageWidth, smallImageHeight, animationDuration,
                                                ZoomAnimationImage::FIXED_LEFT_AND_TOP, EasingEquations::linearEaseNone, EasingEquations::linearEaseNone, EasingEquations::linearEaseNone, EasingEquations::linearEaseNone);
        } else {
            images[i].startZoomAndMoveAnimation(images[i].getX() - smallImageWidth - imagesXPositionDelta, smallImageY, smallImageWidth, smallImageHeight, animationDuration,
                                                ZoomAnimationImage::FIXED_LEFT_AND_TOP, EasingEquations::linearEaseNone, EasingEquations::linearEaseNone, EasingEquations::linearEaseNone, EasingEquations::linearEaseNone);
        }
    }
    currentSelected++;
}

void HorizontalSlideMenu::animateRight()
{
    if (animating() || currentSelected == 0) {
        return;
    }

    for (int i = 0; i < size; i++) {
        // If image is not visible and will not be after animation just move it
        if (images[i].getX() > getWidth()) {
            images[i].moveTo(images[i].getX() + images[i].getWidth() + imagesXPositionDelta, images[i].getY());
        }
        // If image is not visible and will not be after animation just move it
        else if (images[i].getX() + 2 * images[i].getWidth() + imagesXPositionDelta < 0) {
            images[i].moveTo(images[i].getX() + images[i].getWidth() + imagesXPositionDelta, images[i].getY());
        } else if (i == currentSelected - 1) {
            images[i].startZoomAndMoveAnimation(images[i].getX() + smallImageWidth + imagesXPositionDelta, largeImageY, largeImageWidth, largeImageHeight, animationDuration,
                                                ZoomAnimationImage::FIXED_LEFT_AND_TOP, EasingEquations::linearEaseNone, EasingEquations::linearEaseNone, EasingEquations::linearEaseNone, EasingEquations::linearEaseNone);
        } else if (i == currentSelected) {
            images[i].startZoomAndMoveAnimation(images[i].getX() + largeImageWidth + imagesXPositionDelta, smallImageY, smallImageWidth, smallImageHeight, animationDuration,
                                                ZoomAnimationImage::FIXED_LEFT_AND_TOP, EasingEquations::linearEaseNone, EasingEquations::linearEaseNone, EasingEquations::linearEaseNone, EasingEquations::linearEaseNone);
        } else {
            images[i].startZoomAndMoveAnimation(images[i].getX() + smallImageWidth + imagesXPositionDelta, smallImageY, smallImageWidth, smallImageHeight, animationDuration,
                                                ZoomAnimationImage::FIXED_LEFT_AND_TOP, EasingEquations::linearEaseNone, EasingEquations::linearEaseNone, EasingEquations::linearEaseNone, EasingEquations::linearEaseNone);
        }
    }
    currentSelected--;
}

void HorizontalSlideMenu::animationEndedHandler(const ZoomAnimationImage &image)
{
    if (elementSelectedAction && elementSelectedAction->isValid()) {
        elementSelectedAction->execute(*this);
    }
}

bool HorizontalSlideMenu::animating()
{
    for (int i = 0; i < size; i++) {
        if (images[i].isRunning()) {
            return true;
        }
    }
    return false;
}
