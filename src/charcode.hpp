/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_CHARCODE_HPP
#define SASS_CHARCODE_HPP

#include <cstdint>
#include "utf8/core.h"

namespace Sass {

  namespace Charcode {

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    // "Null character" control character.
    const uint8_t $nul = 0x00;

    // "Start of Header" control character.
    const uint8_t $soh = 0x01;

    // "Start of Text" control character.
    const uint8_t $stx = 0x02;

    // "End of Text" control character.
    const uint8_t $etx = 0x03;

    // "End of Transmission" control character.
    const uint8_t $eot = 0x04;

    // "Enquiry" control character.
    const uint8_t $enq = 0x05;

    // "Acknowledgment" control character.
    const uint8_t $ack = 0x06;

    // "Bell" control character.
    const uint8_t $bel = 0x07;

    // "Backspace" control character.
    const uint8_t $bs = 0x08;

    // "Horizontal Tab" control character.
    const uint8_t $tab = 0x09;

    // "Line feed" control character.
    const uint8_t $lf = 0x0A;

    // "Vertical Tab" control character.
    const uint8_t $vt = 0x0B;

    // "Form feed" control character.
    const uint8_t $ff = 0x0C;

    // "Carriage return" control character.
    const uint8_t $cr = 0x0D;

    // "Shift Out" control character.
    const uint8_t $so = 0x0E;

    // "Shift In" control character.
    const uint8_t $si = 0x0F;

    // "Data Link Escape" control character.
    const uint8_t $dle = 0x10;

    // "Device Control 1" control character (oft. XON).
    const uint8_t $dc1 = 0x11;

    // "Device Control 2" control character.
    const uint8_t $dc2 = 0x12;

    // "Device Control 3" control character (oft. XOFF).
    const uint8_t $dc3 = 0x13;

    // "Device Control 4" control character.
    const uint8_t $dc4 = 0x14;

    // "Negative Acknowledgment" control character.
    const uint8_t $nak = 0x15;

    // "Synchronous idle" control character.
    const uint8_t $syn = 0x16;

    // "End of Transmission Block" control character.
    const uint8_t $etb = 0x17;

    // "Cancel" control character.
    const uint8_t $can = 0x18;

    // "End of Medium" control character.
    const uint8_t $em = 0x19;

    // "Substitute" control character.
    const uint8_t $sub = 0x1A;

    // "Escape" control character.
    const uint8_t $esc = 0x1B;

    // "File Separator" control character.
    const uint8_t $fs = 0x1C;

    // "Group Separator" control character.
    const uint8_t $gs = 0x1D;

    // "Record Separator" control character.
    const uint8_t $rs = 0x1E;

    // "Unit Separator" control character.
    const uint8_t $us = 0x1F;

    // "Delete" control character.
    const uint8_t $del = 0x7F;

    /////////////////////////////////////////////////////////////////////////
    // Visible characters.
    /////////////////////////////////////////////////////////////////////////

    // Space character.
    const uint8_t $space = 0x20;

    // Character '!'.
    const uint8_t $exclamation = 0x21;

    // Character '"'.
    const uint8_t $quote = 0x22;

    // Character '#'.
    const uint8_t $hash = 0x23;

    // Character '$'.
    const uint8_t $dollar = 0x24;

    // Character '%'.
    const uint8_t $percent = 0x25;

    // Character '&'.
    const uint8_t $ampersand = 0x26;

    // Character "'".
    const uint8_t $apos = 0x27;

    // Character '('.
    const uint8_t $lparen = 0x28;

    // Character ')'.
    const uint8_t $rparen = 0x29;

    // Character '*'.
    const uint8_t $asterisk = 0x2A;

    // Character '+'.
    const uint8_t $plus = 0x2B;

    // Character ','.
    const uint8_t $comma = 0x2C;

    // Character '-'.
    const uint8_t $minus = 0x2D;

    // Character '.'.
    const uint8_t $dot = 0x2E;

    // Character '/'.
    const uint8_t $slash = 0x2F;

    // Character '0'.
    const uint8_t $0 = 0x30;

    // Character '1'.
    const uint8_t $1 = 0x31;

    // Character '2'.
    const uint8_t $2 = 0x32;

    // Character '3'.
    const uint8_t $3 = 0x33;

    // Character '4'.
    const uint8_t $4 = 0x34;

    // Character '5'.
    const uint8_t $5 = 0x35;

    // Character '6'.
    const uint8_t $6 = 0x36;

    // Character '7'.
    const uint8_t $7 = 0x37;

    // Character '8'.
    const uint8_t $8 = 0x38;

    // Character '9'.
    const uint8_t $9 = 0x39;

    // Character ':'.
    const uint8_t $colon = 0x3A;

    // Character ';'.
    const uint8_t $semicolon = 0x3B;

    // Character '<'.
    const uint8_t $lt = 0x3C;

    // Character '='.
    const uint8_t $equal = 0x3D;

    // Character '>'.
    const uint8_t $gt = 0x3E;

    // Character '?'.
    const uint8_t $question = 0x3F;

    // Character '@'.
    const uint8_t $at = 0x40;

    // Character 'A'.
    const uint8_t $A = 0x41;

    // Character 'B'.
    const uint8_t $B = 0x42;

    // Character 'C'.
    const uint8_t $C = 0x43;

    // Character 'D'.
    const uint8_t $D = 0x44;

    // Character 'E'.
    const uint8_t $E = 0x45;

    // Character 'F'.
    const uint8_t $F = 0x46;

    // Character 'G'.
    const uint8_t $G = 0x47;

    // Character 'H'.
    const uint8_t $H = 0x48;

    // Character 'I'.
    const uint8_t $I = 0x49;

    // Character 'J'.
    const uint8_t $J = 0x4A;

    // Character 'K'.
    const uint8_t $K = 0x4B;

    // Character 'L'.
    const uint8_t $L = 0x4C;

    // Character 'M'.
    const uint8_t $M = 0x4D;

    // Character 'N'.
    const uint8_t $N = 0x4E;

    // Character 'O'.
    const uint8_t $O = 0x4F;

    // Character 'P'.
    const uint8_t $P = 0x50;

    // Character 'Q'.
    const uint8_t $Q = 0x51;

    // Character 'R'.
    const uint8_t $R = 0x52;

    // Character 'S'.
    const uint8_t $S = 0x53;

    // Character 'T'.
    const uint8_t $T = 0x54;

    // Character 'U'.
    const uint8_t $U = 0x55;

    // Character 'V'.
    const uint8_t $V = 0x56;

    // Character 'W'.
    const uint8_t $W = 0x57;

    // Character 'X'.
    const uint8_t $X = 0x58;

    // Character 'Y'.
    const uint8_t $Y = 0x59;

    // Character 'Z'.
    const uint8_t $Z = 0x5A;

    // Character '['.
    const uint8_t $lbracket = 0x5B;

    // Character '\'.
    const uint8_t $backslash = 0x5C;

    // Character ']'.
    const uint8_t $rbracket = 0x5D;

    // Character '^'.
    const uint8_t $circumflex = 0x5E;

    // Character '^'.
    const uint8_t $caret = 0x5E;

    // Character '^'.
    const uint8_t $hat = 0x5E;

    // Character '_'.
    const uint8_t $_ = 0x5F;

    // Character '_'.
    const uint8_t $underscore = 0x5F;

    // Character '_'.
    const uint8_t $underline = 0x5F;

    // Character '`'.
    const uint8_t $backquote = 0x60;

    // Character '`'.
    const uint8_t $grave = 0x60;

    // Character 'a'.
    const uint8_t $a = 0x61;

    // Character 'b'.
    const uint8_t $b = 0x62;

    // Character 'c'.
    const uint8_t $c = 0x63;

    // Character 'd'.
    const uint8_t $d = 0x64;

    // Character 'e'.
    const uint8_t $e = 0x65;

    // Character 'f'.
    const uint8_t $f = 0x66;

    // Character 'g'.
    const uint8_t $g = 0x67;

    // Character 'h'.
    const uint8_t $h = 0x68;

    // Character 'i'.
    const uint8_t $i = 0x69;

    // Character 'j'.
    const uint8_t $j = 0x6A;

    // Character 'k'.
    const uint8_t $k = 0x6B;

    // Character 'l'.
    const uint8_t $l = 0x6C;

    // Character 'm'.
    const uint8_t $m = 0x6D;

    // Character 'n'.
    const uint8_t $n = 0x6E;

    // Character 'o'.
    const uint8_t $o = 0x6F;

    // Character 'p'.
    const uint8_t $p = 0x70;

    // Character 'q'.
    const uint8_t $q = 0x71;

    // Character 'r'.
    const uint8_t $r = 0x72;

    // Character 's'.
    const uint8_t $s = 0x73;

    // Character 't'.
    const uint8_t $t = 0x74;

    // Character 'u'.
    const uint8_t $u = 0x75;

    // Character 'v'.
    const uint8_t $v = 0x76;

    // Character 'w'.
    const uint8_t $w = 0x77;

    // Character 'x'.
    const uint8_t $x = 0x78;

    // Character 'y'.
    const uint8_t $y = 0x79;

    // Character 'z'.
    const uint8_t $z = 0x7A;

    // Character '{'.
    const uint8_t $lbrace = 0x7B;

    // Character '|'.
    const uint8_t $pipe = 0x7C;

    // Character '|'.
    const uint8_t $bar = 0x7C;

    // Character '}'.
    const uint8_t $rbrace = 0x7D;

    // Character '~'.
    const uint8_t $tilde = 0x7E;

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

  }

}

#endif
