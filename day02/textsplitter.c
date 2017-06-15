
#include <stdio.h>

int main(void)
{
    // 1024 characters is a nice large buffer size
    static char buf[1024];

    FILE *even = fopen("output.even", "w");
    if(!even) {
        perror("Could not open 'output.even'");
        return 1;
    }

    FILE *odd = fopen("output.odd", "w");
    if(!odd) {
        fclose(even);
        perror("Could not open 'output.odd'");
        return 1;
    }

    while(!feof(stdin)) {
        if(!fgets(buf, sizeof(buf), stdin)) {
            break;
        }
        fputs(buf, odd);

        if(!fgets(buf, sizeof(buf), stdin)) {
            break;
        }
        fputs(buf, even);
    }

    fclose(even);
    fclose(odd);
}
