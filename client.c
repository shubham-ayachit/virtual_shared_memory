#include "func.h"
#include "string.h"

int main(int argc, char **argv) {

        int loc, num, action;
        char sendline[MEMSIZE];

        //fprintf(stderr, "Enter loc: ");
        //scanf("%d", &loc);
        //fprintf(stderr, "Enter data: ");
        //scanf("%s", &sendline);
        //fprintf(stderr, "Enter num: ");
        //scanf("%d", &num);

        fprintf(stderr, "Enter action: 1 - Write, 2 - Lock, 3 - Unloack, 4 - Lock all, 5 - unloack all, 6 - read : ");
        scanf("%d", &action);

        switch(action) {
                case 1 :
                        fprintf(stderr, "Enter loc: ");
                        scanf("%d", &loc);
                        fprintf(stderr, "Enter num: ");
                        scanf("%d", &num);
                        fprintf(stderr, "Enter data: ");
                        scanf("%s", &sendline);
                        num = WriteNMem(loc, sendline, num);
                        break;
                case 2 :
                        fprintf(stderr, "Enter loc: ");
                        scanf("%d", &loc);
                        fprintf(stderr, "Enter num: ");
                        scanf("%d", &num);
                        num = Llock(loc, num);
                        break;
                case 3 :
                        fprintf(stderr, "Enter loc: ");
                        scanf("%d", &loc);
                        fprintf(stderr, "Enter num: ");
                        scanf("%d", &num);
                        num = Lunlock(loc);
                        break;
                case 4 :
                        num = Glock();
                        break;
                case 5 :
                        num = Gunlock();
                        break;
                case 6 :
                        num = ReadNMem(loc, sendline, num);
                        break;
        }
        printf("%d\n",num);
        printf("%s\n",sendline);

        return 0;
}