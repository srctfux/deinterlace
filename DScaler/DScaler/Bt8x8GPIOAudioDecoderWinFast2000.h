//
// $Id$
//
/////////////////////////////////////////////////////////////////////////////
//
// copyleft 2002 itt@myself.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file bt8x8gpioaudiodecoderwinfast2000.h bt8x8gpioaudiodecoderwinfast2000 Header
 */

#ifdef WANT_BT8X8_SUPPORT

#if !defined(__BT8X8GPIOAUDIODECODERWINFAST2000_H__)
#define __BT8X8GPIOAUDIODECODERWINFAST2000_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Bt8x8GPIOAudioDecoder.h"

class CBt8x8GPIOAudioDecoderWinFast2000 : public CBt8x8GPIOAudioDecoder
{
public:
    CBt8x8GPIOAudioDecoderWinFast2000(CPCICard* pPCICard) :
        CBt8x8GPIOAudioDecoder(pPCICard, 0x430000, 0x020000, 0x420000, 0x410000) {}
};

#endif // !defined(__BT8X8GPIOAUDIODECODERWINFAST2000_H__)

#endif // WANT_BT8X8_SUPPORT
