#include <string.h>
#include <stdint.h>

#include "text.h"

// TODO: This is wasteful, should figure out something more elegant
const char *A0 = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ+-./?";
const char *A1 = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char *A2 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char *A3 = "0123456789";
const char *A4 = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";

int index(const char *string, char c) {
    for (int i = 0; *string; ++i, ++string) {
        if (c == *string) {
            return i;
        }
    }
    return -1;  // Not found
}

// Pack a special token, a 22-bit hash code, or a valid base call 
// into a 28-bit integer.
int32_t pack28(const char *callsign) {
    constexpr int32_t NTOKENS = 2063592L;
    constexpr int32_t MAX22   = 4194304L;

    // Check for special tokens first
    if (starts_with(callsign, "DE ")) return 0;
    if (starts_with(callsign, "QRZ ")) return 1;
    if (starts_with(callsign, "CQ ")) return 2;

    if (starts_with(callsign, "CQ_")) {
        int nnum = 0, nlet = 0;

        // if(nnum.eq.3 .and. nlet.eq.0) then n28=3+nqsy
        // if(nlet.ge.1 .and. nlet.le.4 .and. nnum.eq.0) then n28=3+1000+m
    }

    // Check for <...> callsign
    // if(c13(1:1).eq.'<')then
    //   call save_hash_call(c13,n10,n12,n22)   !Save callsign in hash table
    //   n28=NTOKENS + n22

    char c6[6] = {' ', ' ', ' ', ' ', ' ', ' '};

    int length = 0; // strlen(callsign);  // We will need it later
    while (callsign[length] != ' ' && callsign[length] != 0) {
        length++;
    }

    // Work-around for Swaziland prefix:
    if (starts_with(callsign, "3DA0") && length <= 7) {
        memcpy(c6, "3D0", 3);
        memcpy(c6 + 3, callsign + 4, length - 4);
    }
    // Work-around for Guinea prefixes:
    else if (starts_with(callsign, "3X") && is_letter(callsign[2]) && length <= 7) {
        memcpy(c6, "Q", 1);
        memcpy(c6 + 1, callsign + 2, length - 2);
    }
    else {
        if (is_digit(callsign[2]) && length <= 6) {
            memcpy(c6, callsign, length);
        }
        else if (is_digit(callsign[1]) && length <= 5) {
            memcpy(c6 + 1, callsign, length);
        }
    }

    // Check for standard callsign
    int i0, i1, i2, i3, i4, i5;
    if ((i0 = index(A1, c6[0])) >= 0 && (i1 = index(A2, c6[1])) >= 0 &&
        (i2 = index(A3, c6[2])) >= 0 && (i3 = index(A4, c6[3])) >= 0 &&
        (i4 = index(A4, c6[4])) >= 0 && (i5 = index(A4, c6[5])) >= 0) 
    {
        // This is a standard callsign
        int32_t n28 = i0;
        n28 = n28 * 36 + i1;
        n28 = n28 * 10 + i2;
        n28 = n28 * 27 + i3;
        n28 = n28 * 27 + i4;
        n28 = n28 * 27 + i5;
        return NTOKENS + MAX22 + n28;
    }

    //char c13[13];

    //if (length > 13) return -1;

    // Treat this as a nonstandard callsign: compute its 22-bit hash
    // call save_hash_call(c13,n10,n12,n22)   !Save callsign in hash table
    // n28=NTOKENS + n22

    // n28=iand(n28,ishft(1,28)-1)
    return -1;
}


// Check if a string could be a valid standard callsign or a valid
// compound callsign.
// Return base call "bc" and a logical "cok" indicator.
bool chkcall(const char *call, char *bc) {
    int length = strlen(call);   // n1=len_trim(w)
    if (length > 11) return false;
    if (0 != strchr(call, '.')) return false;
    if (0 != strchr(call, '+')) return false;
    if (0 != strchr(call, '-')) return false;
    if (0 != strchr(call, '?')) return false;
    if (length > 6 && 0 != strchr(call, '/')) return false;

  //bc=w(1:6)
  //i0=index(w,'/')
  //if(max(i0-1,n1-i0).gt.6) go to 100      !Base call must be < 7 characters
  //if(i0.ge.2 .and. i0.le.n1-1) then       !Extract base call from compound call
  //   if(i0-1.le.n1-i0) bc=w(i0+1:n1)//'   '
  //   if(i0-1.gt.n1-i0) bc=w(1:i0-1)//'   '
}


uint16_t packgrid(const char *grid4) {
    constexpr uint16_t MAXGRID4 = 32400;

    if (grid4 == 0) {
        // Two callsigns only, no report/grid
        return MAXGRID4 + 1;
    }

    // Take care of special cases
    if (equals(grid4, "RRR")) return MAXGRID4 + 2;
    if (equals(grid4, "RR73")) return MAXGRID4 + 3;
    if (equals(grid4, "73")) return MAXGRID4 + 4;

    // Check for standard 4 letter grid
    if (in_range(grid4[0], 'A', 'R') && 
        in_range(grid4[1], 'A', 'R') &&
        is_digit(grid4[2]) && is_digit(grid4[3])) 
    {
        //if (w(3).eq.'R ') ir=1
        uint16_t igrid4 = (grid4[3] - '0');
        igrid4 = igrid4 * 10 + (grid4[2] - '0');
        igrid4 = igrid4 * 10 + (grid4[1] - 'A');
        igrid4 = igrid4 * 18 + (grid4[0] - 'A');
        return igrid4;
    }

    // Parse report: +dd / -dd / R+dd / R-dd
    // TODO: check the range of dd
    if (grid4[0] == 'R') {
        int dd = dd_to_int(grid4 + 1, 3);
        uint16_t irpt = 35 + dd;
        return (MAXGRID4 + irpt) | 0x8000;  // ir = 1
    }
    else {
        int dd = dd_to_int(grid4, 3);
        uint16_t irpt = 35 + dd;
        return (MAXGRID4 + irpt);           // ir = 0
    }

    return MAXGRID4 + 1;
}

// Pack Type 1 (Standard 77-bit message) and Type 2 (ditto, with a "/P" call)
int pack77_1(const char *msg, uint8_t *b77) {
    // Locate the first delimiter
    const char *s1 = strchr(msg, ' ');
    if (s1 == 0) return -1;

    const char *call1 = msg;        // 1st call
    const char *call2 = s1 + 1;     // 2nd call

    int32_t n28a = pack28(call1);
    int32_t n28b = pack28(call2);

    uint16_t igrid4;

    // Locate the second delimiter
    const char *s2 = strchr(s1 + 1, ' ');
    if (s2 != 0) {
        igrid4 = packgrid(s2 + 1);
    }
    else {
        // Two callsigns, no grid/report
        igrid4 = packgrid(0);
    }

    uint8_t i3 = 1;
    // if(index(w(1),'/P').ge.4 .or. index(w(2),'/P').ge.4) i3=2  !Type 2, with "/P"

    // if(index(w(1),'/P').ge.4 .or. index(w(1),'/R').ge.4) ipa=1
    // if(index(w(2),'/P').ge.4 .or. index(w(2),'/R').ge.4) ipb=1

    // Shift in ipa and ipb bits into n28a and n28b
    n28a <<= 1; // ipa = 0
    n28b <<= 1; // ipb = 0

    // write(c77,1000) n28a,ipa,n28b,ipb,ir,igrid4,i3
    // 1000 format(2(b28.28,b1),b1,b15.15,b3.3)  
    // (28 + 1) + (28 + 1) + (1 + 15) + 3  

    b77[0] = (n28a >> 21);
    b77[1] = (n28a >> 13);
    b77[2] = (n28a >> 5);
    b77[3] = (uint8_t)(n28a << 3) | (uint8_t)(n28b >> 26);
    b77[4] = (n28b >> 18);
    b77[5] = (n28a >> 10);
    b77[6] = (n28a >> 2);
    b77[7] = (uint8_t)(n28a << 6) | (uint8_t)(igrid4 >> 10);
    b77[8] = (igrid4 >> 2);
    b77[9] = (uint8_t)(igrid4 << 6) | (uint8_t)(i3 << 3);

    return 0;
}


void packtext77(const char *c13, uint8_t *b71) {
    // TODO: w=adjustr(c13)

    for (int i = 0; i < 9; ++i) {
        b71[i] = 0;
    }

    for (int j = 0; j < 13; ++j) {
        int q = index(A0, c13[j]);

        // Multiply b71 by 42
        uint16_t x = 0;
        for (int i = 8; i >= 0; --i) {
            x += b71[i] * (uint16_t)42;
            b71[i] = x;
            x >>= 8;
        }

        // Add index of the current char
        x = (q > 0) ? q : 0;
        for (int i = 8; i >= 0; --i) {
            if (x == 0) break;
            x += b71[i];
            b71[i] = x;
            x >>= 8;
        }
    }
}

int pack77(const char *msg, uint8_t *c77) {
    // Check Type 1 (Standard 77-bit message) or Type 2, with optional "/P"
    //if (starts_with(msg, "CQ ")) {
    return pack77_1(msg, c77);
    //}
    // Check 0.5 (telemetry)
    // i3=0 n3=5 write(c77,1006) ntel,n3,i3 1006 format(b23.23,2b24.24,2b3.3)

    // Check Type 4 (One nonstandard call and one hashed call)
    // pack77_4(nwords,w,i3,n3,c77)

    // Default to free text
    // i3=0 n3=0
    // packtext77(msg(1:13),c77(1:71))
    // write(c77(72:77),'(2b3.3)') n3,i3
}