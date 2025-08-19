#include "../headers/sourparse.h"

int main(){
    SP_font *cal_sans = SP_load_font("fonts/CalSans-Regular.ttf");
    // SP_font *jetbrains_mono = SP_load_font("fonts/JetBrainsMono-Regular.ttf");

    SP_free_font(cal_sans);
    // SP_free_font(jetbrains_mono);

    return 0;
}