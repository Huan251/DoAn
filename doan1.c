#include <16F887.h>
#device *=16 adc=8
#FUSES NOWDT, HS, NOPUT, NODEBUG, NOBROWNOUT, NOLVP, NOCPD, NOWRT
#use delay(clock = 20000000)

// I2C master dùng cho LCD
#use i2c(MASTER, SDA=PIN_C4, SCL=PIN_C3, FORCE_SW)

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
// LCD I2C
#include "I2C_LCD.c"   // nhúng thý vi?n b?n ð? g?i

#define LCD_ADDR 0x4E
// ================== MA TR?N 4x4 ==================
#define HANG_1 PIN_B0
#define HANG_2 PIN_B1
#define HANG_3 PIN_B2
#define HANG_4 PIN_B3

#define COT_1 PIN_B4
#define COT_2 PIN_B5
#define COT_3 PIN_B6
#define COT_4 PIN_B7


// LAYOUT PHÍM
const char PHIM_THUONG[4][4] = {
    {'7','8','9','/'}, 
    {'4','5','6','*'}, 
    {'1','2','3','-'}, 
    {'S','0','=','+'}
};

const char PHIM_SHIFT[4][4] = {
    {'C','A','I','J'},
    {'!','^','D','|'},
    {'(',')','Q','X'},
    {'.','.','%','V'}
};

// ================== BI?N TOÁN TÍNH ==================

signed long int ANS = 0;
signed long int luu_gia_tri_I = 0;
signed long int luu_gia_tri_J = 0;
long int loi = 0;
int1 shift_mode = 0;

void cap_nhat_shift_lcd() {
    if(shift_mode) {
        LCD_Goto(16,1);
        LCD_Out('S');
    } else {
        LCD_Goto(16,1);
        LCD_Out(' ');
    }
}
// ================== KHAI BÁO HÀM ==================
char doc_phim() {
    int8 hang, cot;
    char phim = 0;

    for(hang = 0; hang < 4; hang++) {

        output_high(HANG_1);
        output_high(HANG_2);
        output_high(HANG_3);
        output_high(HANG_4);

        switch(hang) {
            case 0: output_low(HANG_1); break;
            case 1: output_low(HANG_2); break;
            case 2: output_low(HANG_3); break;
            case 3: output_low(HANG_4); break;
        }

        delay_us(10);

        for(cot = 0; cot < 4; cot++) {
            int1 nhan = 0;

            switch(cot) {
                case 0: nhan = !input(COT_1); break;
                case 1: nhan = !input(COT_2); break;
                case 2: nhan = !input(COT_3); break;
                case 3: nhan = !input(COT_4); break;
            }

            if(nhan) {

    // ===== PHÍM SHIFT (V? TRÍ C? Ð?NH) =====
    if(PHIM_THUONG[hang][cot] == 'S') {
        return 'S';
    }

    // ===== PHÍM KHÁC =====
    if(shift_mode)
        phim = PHIM_SHIFT[hang][cot];
    else
        phim = PHIM_THUONG[hang][cot];

    return phim;
}

        }
    }
    return 0;
}


// ============= CÁC HÀM TÍNH TOÁN (GI? NGUYÊN 100%) =============

signed long uu_tien_toan_tu(char toan_tu) {
    if (toan_tu == '+' || toan_tu == '-') return 1;
    if (toan_tu == '*' || toan_tu == '/' || toan_tu == '%') return 2;
    if (toan_tu == '^') return 3;
    if (toan_tu == '!') return 4;
    return 0;
}

signed long phep_toan(char c) {
    return (c == '+' || c == '-' || c == '*' || 
            c == '/' || c == '^' || c == '%');
}

signed long  tinh_luy_thua(long int co_so, long int so_mu) {
    if (co_so == 0){
      if (so_mu > 0) return 0;
      else if (so_mu == 0) return 1;
      else {
         loi = 1;
         return 0;
      }
    }
    else if (co_so == 1) return 1;
    else if (co_so == -1) {
        if (so_mu % 2 == 0) return 1;
        else return -1;
    }
    if(so_mu < 0) return 0;
    signed long  ket_qua = 1;
    if (co_so != 0 && so_mu > 0) {
        if (so_mu >= 15 && co_so >= 2) {
            loi = 2;
            return 0;
        }
        if (so_mu >= 1 && co_so > 181) {
            loi = 2;
            return 0;
        }
    }
    for (long int i = 0; i < so_mu; i++) {
        if (co_so != 0 && (long int)
        
        767 / ABS(co_so) < ABS(ket_qua)) {
            loi = 2;
            return 0;
        }
        ket_qua *= co_so;
    }
    return ket_qua;
}

signed long tinh_toan(long int so_a, long int so_b, char toan_tu) {
    switch (toan_tu) {
        case '+': return so_a + so_b;
        case '-': return so_a - so_b;
        case '*': return so_a * so_b;
        case '/':
            if (so_b == 0) {
                loi = 1;
                return 0;
            }
            return so_a / so_b;

        case '%':
            if (so_b == 0) {
                loi = 1;   
                return 0;
            }
            return so_a % so_b;

        case '^': return tinh_luy_thua(so_a, so_b);
    }
    return 0;
}


signed long  tinh_giai_thua(long int so_n) {
    if (so_n < 0) {
        loi = 1;
        return 0;
    }
    if (so_n == 0 || so_n == 1) return 1;
    if (so_n > 7) {
        loi = 2;
        return 0;
    }
    signed long  ket_qua = 1;
    for (long int i = 2; i <= so_n; i++) {
        ket_qua *= i;
    }
    return ket_qua;
}

signed long tinh_sqrt(signed long n) {
    if (n < 0) { loi = 1; return 0; }
    if (n <= 1) return n;
    
    signed long x = n / 2;
    signed long prev;
    
    do {
        prev = x;
        x = (x + n / x) / 2;
    } while (x < prev);
    
    return prev;
}

signed long tinh_bieu_thuc(char *bieu_thuc) {
    signed long val[20];
    char op[20];
    int vtop = -1, otop = -1;
    signed long i;

    loi = 0;

    for (i = 0; i < strlen(bieu_thuc); i++) {
        char c = bieu_thuc[i];
        if (c == ' ') continue;

        /* ===== X? L? 'A' (ANS) ===== */
       /* ===== X? L? 'A' (ANS) ===== */
if (c == 'A') {
    // N?u trý?c A là s? ho?c ) ho?c A/I/J ? thêm phép nhân ng?m
    if (i > 0) {
        char prev = bieu_thuc[i-1];
        if ((prev >= '0' && prev <= '9') || prev == ')' || 
            prev == 'A' || prev == 'I' || prev == 'J') {
            while (otop != -1 && op[otop] != '(' &&
                   uu_tien_toan_tu(op[otop]) >= uu_tien_toan_tu('*')) {
                if (vtop < 1) { loi = 1; return 0; }
                signed long b = val[vtop--];
                signed long a = val[vtop--];
                val[++vtop] = tinh_toan(a, b, op[otop--]);
                if (loi) return 0;
            }
            op[++otop] = '*';
        }
    }
    val[++vtop] = ANS;
    continue;
}

        /* ===== X? L? 'I' ===== */
       /* ===== X? L? 'I' ===== */
if (c == 'I') {
    // N?u trý?c I là s? ho?c ) ho?c A/I/J ? thêm phép nhân ng?m
    if (i > 0) {
        char prev = bieu_thuc[i-1];
        if ((prev >= '0' && prev <= '9') || prev == ')' || 
            prev == 'A' || prev == 'I' || prev == 'J') {
            while (otop != -1 && op[otop] != '(' &&
                   uu_tien_toan_tu(op[otop]) >= uu_tien_toan_tu('*')) {
                if (vtop < 1) { loi = 1; return 0; }
                signed long b = val[vtop--];
                signed long a = val[vtop--];
                val[++vtop] = tinh_toan(a, b, op[otop--]);
                if (loi) return 0;
            }
            op[++otop] = '*';
        }
    }
    val[++vtop] = luu_gia_tri_I;
    continue;
}

        /* ===== X? L? 'J' ===== */
       /* ===== X? L? 'J' ===== */
if (c == 'J') {
    // N?u trý?c J là s? ho?c ) ho?c A/I/J ? thêm phép nhân ng?m
    if (i > 0) {
        char prev = bieu_thuc[i-1];
        if ((prev >= '0' && prev <= '9') || prev == ')' || 
            prev == 'A' || prev == 'I' || prev == 'J') {
            while (otop != -1 && op[otop] != '(' &&
                   uu_tien_toan_tu(op[otop]) >= uu_tien_toan_tu('*')) {
                if (vtop < 1) { loi = 1; return 0; }
                signed long b = val[vtop--];
                signed long a = val[vtop--];
                val[++vtop] = tinh_toan(a, b, op[otop--]);
                if (loi) return 0;
            }
            op[++otop] = '*';
        }
    }
    val[++vtop] = luu_gia_tri_J;
    continue;
}

        /* ===== TOÁN T? ÐÕN '-' ===== */
        if (c == '-' && (i == 0 || bieu_thuc[i-1] == '(' || phep_toan(bieu_thuc[i-1]))) {
            val[++vtop] = 0;
            op[++otop] = '-';
            continue;
        }

        /* ===== S? ===== */
        if (c >= '0' && c <= '9') {
            signed long num = 0;
            while (i < strlen(bieu_thuc) && bieu_thuc[i] >= '0' && bieu_thuc[i] <= '9') {
                num = num * 10 + (bieu_thuc[i] - '0');
                i++;
            }
            val[++vtop] = num;
            i--;
        }

        /* ===== NGO?C ===== */
        else if (c == '(') {
            op[++otop] = '(';
        }
        else if (c == ')') {
            while (otop != -1 && op[otop] != '(') {
                if (vtop < 1) { loi = 1; return 0; }
                signed long b = val[vtop--];
                signed long a = val[vtop--];
                val[++vtop] = tinh_toan(a, b, op[otop--]);
                if (loi) return 0;
            }
            if (otop == -1) { loi = 1; return 0; }
            otop--;
            
            // N?u sau ) là A, I, ho?c J ? thêm phép nhân
            if (i+1 < strlen(bieu_thuc)) {
                char next = bieu_thuc[i+1];
                if (next == 'A' || next == 'I' || next == 'J') {
                    op[++otop] = '*';
                }
            }
        }

        /* ===== GIAI TH?A ===== */
        else if (c == '!') {
            if (vtop < 0) {
                loi = 1;
                return 0;
            }
            if (val[vtop] < 0) {
                loi = 1;
                return 0;
            }
            val[vtop] = tinh_giai_thua(val[vtop]);
            if (loi) return 0;
        }

        /* ===== TOÁN T? ===== */
        else if (phep_toan(c)) {
            if (c == '^') {
                while (otop != -1 && op[otop] != '(' &&
                       uu_tien_toan_tu(op[otop]) > uu_tien_toan_tu('^')) {
                    if (vtop < 1) { loi = 1; return 0; }
                    signed long b = val[vtop--];
                    signed long a = val[vtop--];
                    val[++vtop] = tinh_toan(a, b, op[otop--]);
                    if (loi) return 0;
                }
            } else {
                while (otop != -1 && op[otop] != '(' &&
                       uu_tien_toan_tu(op[otop]) >= uu_tien_toan_tu(c)) {
                    if (vtop < 1) { loi = 1; return 0; }
                    signed long b = val[vtop--];
                    signed long a = val[vtop--];
                    val[++vtop] = tinh_toan(a, b, op[otop--]);
                    if (loi) return 0;
                }
            }
            op[++otop] = c;
        }
        else {
            loi = 1;
            return 0;
        }
    }

    while (otop != -1) {
        if (vtop < 1) { loi = 1; return 0; }
        signed long b = val[vtop--];
        signed long a = val[vtop--];
        val[++vtop] = tinh_toan(a, b, op[otop--]);
        if (loi) return 0;
    }

    if (vtop == 0) return val[0];
    loi = 1;
    return 0;
}

int la_mu_chan(char *bt) {
    int i = 0;
    while (bt[i] && bt[i] != '^') i++;
    if (bt[i] != '^') return 1; // không có m?
    i++;
    int mu = 0;
    while (bt[i] >= '0' && bt[i] <= '9') {
        mu = mu * 10 + (bt[i] - '0');
        i++;
    }
    return (mu % 2 == 0);
}


// ================== MAIN ==================
void main() {
    delay_ms(500);
    setup_adc_ports(NO_ANALOGS);
    setup_adc(ADC_OFF);
    char phim;
    char bieu_thuc[15];
    int8 vi_tri = 0;
    int1 da_xong = 0;
    int1 man_hinh_ban_dau = 1;
    char buf[12];

    set_tris_a(0xFF);
    set_tris_b(0xF0);
    set_tris_c(0xFF);
  
    set_tris_e(0xFF);
    port_b_pullups(TRUE);

    LCD_Begin(LCD_ADDR);
    LCD_Cmd(LCD_CLEAR);

    // màn h?nh ban ð?u
    LCD_Goto(1,1);
   LCD_Out('X'); LCD_Out('I'); LCD_Out('N'); LCD_Out(' ');
   LCD_Out('C'); LCD_Out('H'); LCD_Out('A'); LCD_Out('O');
   LCD_Goto(1,2);
    bieu_thuc[0] = '\0';
    while(doc_phim() != 0);

    while(TRUE) {
        phim = doc_phim();
        if(phim) {
            while(doc_phim() == phim);
            delay_ms(200);
            if(phim == 'S') {

    // >>> THÊM ÐO?N NÀY <<<
    if(man_hinh_ban_dau) {
        LCD_Goto(1,1); for(int i=0;i<8;i++) LCD_Out(' ');
        man_hinh_ban_dau = 0;
    }

    shift_mode = !shift_mode;
    cap_nhat_shift_lcd();

    while(doc_phim() == 'S');
    continue;
}


// ===== T?T SHIFT SAU 1 PHÍM =====
if(shift_mode) {
    shift_mode = 0;
    cap_nhat_shift_lcd();
}
            // xóa màn h?nh ban ð?u khi b?m phím ð?u tiên
            if(man_hinh_ban_dau && phim != 'C') {
                LCD_Goto(1,1); for(int i=0;i<16;i++) LCD_Out(' ');
                LCD_Goto(1,2); for(int i=0;i<16;i++) LCD_Out(' ');
                man_hinh_ban_dau = 0;
            }
            // CLEAR
           if(phim == 'C') {
       shift_mode = 0;
       cap_nhat_shift_lcd();
   
       LCD_Cmd(LCD_CLEAR);
       cap_nhat_shift_lcd();
   
       LCD_Goto(1,1);
       LCD_Out('X'); LCD_Out('I'); LCD_Out('N'); LCD_Out(' ');
       LCD_Out('C'); LCD_Out('H'); LCD_Out('A'); LCD_Out('O');
   
       vi_tri = 0;
       bieu_thuc[0] = '\0';
       da_xong = 0;
       man_hinh_ban_dau = 1;
   }

            // BACKSPACE
            else if(phim == 'D') {
                if(vi_tri > 0) {
                    vi_tri--;
                    bieu_thuc[vi_tri] = '\0';
                    LCD_Goto(vi_tri + 1, 1);
                    LCD_Out(' ');
                    LCD_Goto(vi_tri + 1, 1);
                }
            }

            // I
           else if(phim == 'I') {
    if(da_xong) {
        // KI?M TRA L?I TRÝ?C KHI LÝU
        if(loi != 0) {
            // Hi?n th? ERR
            LCD_Cmd(LCD_CLEAR);
            LCD_Goto(1,1);
            LCD_Out('E'); LCD_Out('R'); LCD_Out('R');
            delay_ms(1000);
            
            // Kích ho?t phím C (Clear)
            shift_mode = 0;
            cap_nhat_shift_lcd();
            
            LCD_Cmd(LCD_CLEAR);
            cap_nhat_shift_lcd();
            
            LCD_Goto(1,1);
            LCD_Out('X'); LCD_Out('I'); LCD_Out('N'); LCD_Out(' ');
            LCD_Out('C'); LCD_Out('H'); LCD_Out('A'); LCD_Out('O');
            
            vi_tri = 0;
            bieu_thuc[0] = '\0';
            da_xong = 0;
            man_hinh_ban_dau = 1;
        } else {
            luu_gia_tri_I = ANS;
            LCD_Cmd(LCD_CLEAR);
            LCD_Goto(1,1);
            LCD_Out('L'); LCD_Out('U'); LCD_Out('U'); LCD_Out(' ');
            LCD_Out('I');
            delay_ms(1000);
            LCD_Cmd(LCD_CLEAR);
            man_hinh_ban_dau = 1;
            vi_tri = 0;
            da_xong = 0;
        }
    } else {
        if(da_xong) {
            LCD_Cmd(LCD_CLEAR);
            vi_tri = 0;
            bieu_thuc[0] = '\0';
            da_xong = 0;
        }
        if(vi_tri < 14) {
            bieu_thuc[vi_tri++] = 'I';
            bieu_thuc[vi_tri] = '\0';
            LCD_Goto(vi_tri,1);
            LCD_Out('I');
        }
    }
}

            // J
else if(phim == 'J') {
    if(da_xong) {
        // KI?M TRA L?I TRÝ?C KHI LÝU
        if(loi != 0) {
            // Hi?n th? ERR
            LCD_Cmd(LCD_CLEAR);
            LCD_Goto(1,1);
            LCD_Out('E'); LCD_Out('R'); LCD_Out('R');
            delay_ms(1000);
            
            // Kích ho?t phím C (Clear)
            shift_mode = 0;
            cap_nhat_shift_lcd();
            
            LCD_Cmd(LCD_CLEAR);
            cap_nhat_shift_lcd();
            
            LCD_Goto(1,1);
            LCD_Out('X'); LCD_Out('I'); LCD_Out('N'); LCD_Out(' ');
            LCD_Out('C'); LCD_Out('H'); LCD_Out('A'); LCD_Out('O');
            
            vi_tri = 0;
            bieu_thuc[0] = '\0';
            da_xong = 0;
            man_hinh_ban_dau = 1;
        } else {
            luu_gia_tri_J = ANS;
            LCD_Cmd(LCD_CLEAR);
            LCD_Goto(1,1);
            LCD_Out('L'); LCD_Out('U'); LCD_Out('U'); LCD_Out(' ');
            LCD_Out('J');
            delay_ms(1000);
            LCD_Cmd(LCD_CLEAR);
            man_hinh_ban_dau = 1;
            vi_tri = 0;
            da_xong = 0;
        }
    } else {
        if(da_xong) {
            LCD_Cmd(LCD_CLEAR);
            vi_tri = 0;
            bieu_thuc[0] = '\0';
            da_xong = 0;
        }
        if(vi_tri < 14) {
            bieu_thuc[vi_tri++] = 'J';
            bieu_thuc[vi_tri] = '\0';
            LCD_Goto(vi_tri,1);
            LCD_Out('J');
        }
    }
}


// V (View All - Xem t?t c? bi?n)
else if(phim == 'V') {
    LCD_Cmd(LCD_CLEAR);
    LCD_Goto(1,1);
    // D?ng 1: A 
    sprintf(buf, "A=%ld", ANS);
    for(int i=0; buf[i] && i<7; i++) LCD_Out(buf[i]);
    LCD_Out(' ');
    
    
    // D?ng 2: I và J
    LCD_Goto(1,2);
    sprintf(buf, "I=%ld ;", luu_gia_tri_I);
    for(int i=0; buf[i] && i<7; i++) LCD_Out(buf[i]);
    LCD_Out(' ');
    sprintf(buf, "J=%ld", luu_gia_tri_J);
    for(int i=0; buf[i]; i++) LCD_Out(buf[i]);
    
    // Ð?I B?M PHÍM B?T K?
    while(doc_phim() == 0);  // Ð?i có phím ðý?c b?m
    
    // Xóa màn h?nh và quay l?i
    LCD_Cmd(LCD_CLEAR);
    
    if(da_xong) {
        // N?u v?a tính xong, hi?n th? l?i k?t qu?
        LCD_Goto(1,1);
        LCD_Out('K'); LCD_Out('Q');
        LCD_Goto(1,2);
        LCD_Out('=');
        if(loi == 0) {
            sprintf(buf, "%ld", ANS);
            for(int i=0; buf[i]; i++) LCD_Out(buf[i]);
        } else {
            LCD_Out('E'); LCD_Out('R'); LCD_Out('R');
        }
    } else if(vi_tri > 0) {
        // N?u ðang nh?p, hi?n th? l?i bi?u th?c
        LCD_Goto(1,1);
        for(int i=0; i<vi_tri; i++) {
            LCD_Out(bieu_thuc[i]);
        }
    } else {
        // Màn h?nh tr?ng, quay v? màn h?nh ban ð?u
        man_hinh_ban_dau = 1;
    }
    
}
// ABS (|)
else if(phim == '|') {
    signed long kq;
    
    if(da_xong) {
        kq = ANS;
    } else if(vi_tri > 0) {
        kq = tinh_bieu_thuc(bieu_thuc);
        if(loi != 0) {
            LCD_Cmd(LCD_CLEAR);
            LCD_Goto(1,1);
            LCD_Out('E'); LCD_Out('R'); LCD_Out('R');
            delay_ms(1000);
            continue;
        }
        da_xong = 1;
    } else {
        continue;
    }
    
    if(kq < 0) kq = -kq;
    ANS = kq;
    
    // Hi?n th?
    LCD_Cmd(LCD_CLEAR);
    LCD_Goto(1,1);
    LCD_Out('A'); LCD_Out('B'); LCD_Out('S');
    LCD_Goto(1,2);
    LCD_Out('=');
    sprintf(buf, "%ld", ANS);
    for(int i=0; buf[i]; i++) LCD_Out(buf[i]);
}

// SQRT (Q)
else if(phim == 'Q') {
    signed long kq;
    
    if(da_xong) {
        kq = ANS;
    } else if(vi_tri > 0) {
        kq = tinh_bieu_thuc(bieu_thuc);
        if(loi != 0) {
            LCD_Cmd(LCD_CLEAR);
            LCD_Goto(1,1);
            LCD_Out('E'); LCD_Out('R'); LCD_Out('R');
            delay_ms(1000);
            continue;
        }
        da_xong = 1;
    } else {
        continue;
    }
    
    if(kq < 0) {
        LCD_Cmd(LCD_CLEAR);
        LCD_Goto(1,1);
        LCD_Out('E'); LCD_Out('R'); LCD_Out('R');
        LCD_Goto(1,2);
        LCD_Out('S'); LCD_Out('Q'); LCD_Out('R'); LCD_Out('T');
        LCD_Out('<'); LCD_Out('0');
        delay_ms(1500);
        continue;
    }
    
    ANS = tinh_sqrt(kq);
    
    // Hi?n th?
    LCD_Cmd(LCD_CLEAR);
    LCD_Goto(1,1);
    LCD_Out('S'); LCD_Out('Q'); LCD_Out('R'); LCD_Out('T');
    LCD_Goto(1,2);
    LCD_Out('=');
    sprintf(buf, "%ld", ANS);
    for(int i=0; buf[i]; i++) LCD_Out(buf[i]);
}
            // ANS
           // ANS (hi?n th? ch? A thay v? s?)
// ANS (hi?n th? ch? A thay v? s?)
else if(phim == 'A') {
    // N?u v?a tính xong, clear màn h?nh
    if(da_xong) {
        LCD_Cmd(LCD_CLEAR);
        vi_tri = 0;
        bieu_thuc[0] = '\0';
        da_xong = 0;
    }
    
    // Thêm k? t? 'A' vào bi?u th?c
    if(vi_tri < 14) {
        bieu_thuc[vi_tri++] = 'A';
        bieu_thuc[vi_tri] = '\0';
        LCD_Goto(vi_tri,1);
        LCD_Out('A');
    }
}

else if(phim == 'X') {
    ANS = 0;
    luu_gia_tri_I = 0;
    luu_gia_tri_J = 0;
    
    LCD_Cmd(LCD_CLEAR);
    LCD_Goto(1,1);
    LCD_Out('C'); LCD_Out('L'); LCD_Out('R'); LCD_Out(' ');
    LCD_Out('M'); LCD_Out('E'); LCD_Out('M');
    delay_ms(1000);
    
    LCD_Cmd(LCD_CLEAR);
    LCD_Goto(1,1);
    LCD_Out('X'); LCD_Out('I'); LCD_Out('N'); LCD_Out(' ');
    LCD_Out('C'); LCD_Out('H'); LCD_Out('A'); LCD_Out('O');
    
    vi_tri = 0;
    bieu_thuc[0] = '\0';
    da_xong = 0;
    man_hinh_ban_dau = 1;
}



// ! (GIAI TH?A SAU KHI =)
else if(phim == '!' && da_xong) {

    // clear màn h?nh k?t qu?
    LCD_Cmd(LCD_CLEAR);

    // ðýa ANS vào l?i bi?u th?c
    sprintf(buf, "%ld", ANS);
    vi_tri = 0;
    for(int i = 0; buf[i] && vi_tri < 14; i++) {
        bieu_thuc[vi_tri++] = buf[i];
        bieu_thuc[vi_tri] = '\0';
        LCD_Goto(vi_tri, 1);
        LCD_Out(buf[i]);
    }

    // thêm d?u !
    if(vi_tri < 14) {
        bieu_thuc[vi_tri++] = '!';
        bieu_thuc[vi_tri] = '\0';
        LCD_Goto(vi_tri, 1);
        LCD_Out('!');
    }

    da_xong = 0;   // quay v? tr?ng thái nh?p
}


            // =
           else if(phim == '=') {
    if(vi_tri > 0) {
        int am_trong_ngoac = 0;
        if(bieu_thuc[0] == '(' && bieu_thuc[1] == '-') {
            am_trong_ngoac = 1;
        }

        long int kq = tinh_bieu_thuc(bieu_thuc);

        LCD_Cmd(LCD_CLEAR);
        LCD_Goto(1,1);
        LCD_Out('K'); LCD_Out('Q');  // Ð?i t? KET QUA ? KQ
        LCD_Goto(1,2);
        LCD_Out('=');

        if(loi == 0) {
            if(am_trong_ngoac && !la_mu_chan(bieu_thuc)) {
                kq = 0 - kq;
            }
            sprintf(buf, "%ld", kq);
            for(int i=0; buf[i]; i++) LCD_Out(buf[i]);
            ANS = kq;
        } else {
            LCD_Out('E'); LCD_Out('R'); LCD_Out('R');
        }
        da_xong = 1;
    }
}


            // TOÁN T? SAU KHI =
            else if(da_xong && (phim=='+' || phim=='-' || phim=='*' || phim=='/')) {
                LCD_Cmd(LCD_CLEAR);
                sprintf(buf, "%ld", ANS);
                vi_tri = 0;
                for(int i=0; buf[i] && vi_tri < 14; i++) {
                    bieu_thuc[vi_tri++] = buf[i];
                    bieu_thuc[vi_tri] = '\0';
                    LCD_Goto(vi_tri,1);
                    LCD_Out(buf[i]);
                }
                if(vi_tri < 14) {
                    bieu_thuc[vi_tri++] = phim;
                    bieu_thuc[vi_tri] = '\0';
                    LCD_Goto(vi_tri,1);
                    LCD_Out(phim);
                }
                da_xong = 0;
            }

            // NH?P B?NH THÝ?NG
            else {
                if(da_xong) {
                    LCD_Cmd(LCD_CLEAR);
                    vi_tri = 0;
                    bieu_thuc[0] = '\0';
                    da_xong = 0;
                }
                if(vi_tri < 14) {
                    bieu_thuc[vi_tri++] = phim;
                    bieu_thuc[vi_tri] = '\0';
                    LCD_Goto(vi_tri,1);
                    LCD_Out(phim);
                }
            }
        }
    }
}

