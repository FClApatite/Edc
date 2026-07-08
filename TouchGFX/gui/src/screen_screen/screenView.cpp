#include <gui/screen_screen/screenView.hpp>

screenView::screenView()
{

}

void screenView::setupScreen()
{
    screenViewBase::setupScreen();
}

void screenView::tearDownScreen()
{
    screenViewBase::tearDownScreen();
}
void printff(float value,uint16_t* buffer){
	uint8_t size=10;
    char sign = ' ';
    int int_part;
    int frac_part;
    if (value < 0) {
        sign = '-';
        value = -value; 
    }
    if (value > 3999.9f) {
        Unicode::snprintf(buffer, size, " OL");
        return;
    }

    int_part = (int)value;  // 0-399
    frac_part = (int)((value - int_part) * 100.0f + 0.5f);  
 
    if (frac_part >= 100) {
        frac_part = 0;
        int_part++;
        if (int_part > 199) {
            Unicode::snprintf(buffer, size, " OL");
            return;
        }
    }
    if (sign == '-') {
        if (int_part >= 1000) {
                Unicode::snprintf(buffer, size, "%d", int_part);
        } else if (int_part >= 100) {
            Unicode::snprintf(buffer, size, "%3d.%01d", int_part, frac_part);
        } else if (int_part >= 10){
            Unicode::snprintf(buffer, size, "%2d.%02d", int_part, frac_part);
        } else{
					 Unicode::snprintf(buffer, size, "  %1d.%02d", int_part, frac_part);
         }
    } else {
        if (int_part >= 1000) {
                 Unicode::snprintf(buffer, size, "%d", int_part);
        } else if (int_part >= 100) {
            Unicode::snprintf(buffer, size, "%3d.%01d", int_part, frac_part);
        } else if (int_part >= 10){
            Unicode::snprintf(buffer, size, "%2d.%02d", int_part, frac_part);
        } else{
					 Unicode::snprintf(buffer, size, "  %1d.%02d", int_part, frac_part);
         }
			 }
}
		

extern uint16_t param1;
extern uint16_t second_param;
extern float disp_volt;
extern float disp_amp;
extern float disp_watt;
extern float disp_set;
uint16_t fl_int,fl_deci;
uint16_t iiiii=0;
uint16_t txtBuffer1[20];
uint16_t txtBuffer2[20];
uint16_t txtBuffer3[10];
uint16_t txtBuffer4[10];
uint16_t txtBuffer5[10];
uint16_t tmpBuffer[10];
uint16_t txtBuffer6[5];
extern bool onoffstates;
void screenView::AlterWildcards(){
	printff(disp_set,tmpBuffer);
	int32_t disp_int=disp_set;
	if(param1==0)Unicode::snprintf(txtBuffer1, 20, "CC%sA",&tmpBuffer);
	if(param1==1)Unicode::snprintf(txtBuffer1, 20, "CP%dW",disp_int);
	if(param1==2)Unicode::snprintf(txtBuffer1, 20, "CR%sR",&tmpBuffer[2]);
	if(param1==3){
		switch(second_param){
			case 3:
		Unicode::snprintf(txtBuffer1, 10, "SD"); break;
			case 0:
		Unicode::snprintf(txtBuffer1, 10, "VAR"); break;	
			case 1:
		Unicode::snprintf(txtBuffer1, 10, "SIN"); break;
			case 2:
		Unicode::snprintf(txtBuffer1, 10, "TRIANG"); break;
		}
	}
	if(onoffstates==0){
		Unicode::snprintf(txtBuffer6, 5, "OFF");
	}else if(onoffstates==1){
		Unicode::snprintf(txtBuffer6, 5, "SET");
	}
		
	Unicode::snprintf(txtBuffer2, 10, "0");	
	printff(disp_amp,txtBuffer3);
	printff(disp_volt,txtBuffer4);
	printff(disp_watt,txtBuffer5);
	iiiii++;
	textArea12.setWildcard(txtBuffer6);
textArea10.setWildcard(txtBuffer1);
	textArea8.setWildcard(txtBuffer2);
		textArea5.setWildcard(txtBuffer3);
		textArea6.setWildcard(txtBuffer4);
		textArea7.setWildcard(txtBuffer5);

textArea10.invalidate();
	textArea8.invalidate();
	textArea5.invalidate();
	textArea6.invalidate();
	textArea7.invalidate();
	textArea12.invalidate();
}