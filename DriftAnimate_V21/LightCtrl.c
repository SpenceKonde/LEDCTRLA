#include <LightCtrl_RevF.h>

void init_BL_PWM() {
  takeOverTCA1();
  pinModeFast(LCD_BL_R, OUTPUT);
  pinModeFast(LCD_BL_G, OUTPUT);
  pinModeFast(LCD_BL_B, OUTPUT);
  TCA1.SINGLE.PER = 0xFFFF; // 40 MHz system clock, no prescale -> 610 Hz PWM. Should be fine.
  TCA1.SINGLE.CMP0 = 0x8000;
  TCA1.SINGLE.CMP1 = 0x8000;
  TCA1.SINGLE.CMP1 = 0x8000;
  TCA1.SINGLE.CTRLB = 0x73; // Enable all compare channels, single slope PWM mode.
  TCA1.SINGLE.CTRLA = 1; //enable timer

}

void set_BL_PWM(uint16_t red, uint16_t green, uint16_t blue) {
  volatile uint16_t *ptr;
  ptr = (volatile uint8_t *)((uint16_t)(&TCA1_SINGLE_CMP0BUFL));
  (*ptr++) = red;
  (*ptr++) = green;
  (*ptr++) = blue;
}
bool init_UI_Inputs() {
  uint8_t PortC_OK = 0;
  if (!MVIO.STATUS) {
    uint32_t start = millis();
    while (millis() - start < 2000) { //wait up to 2 seconds
      if (MVIO.STATUS) {
        PortC_OK = 1;
        break;
      }
    }
  } else {
    PortC_OK = 1; // MVIO was already up and running
  }
  if (!PortC_OK) {
    Serial.println("FATAL ERROR - No power to VDDIO2?");
    ErrorSet(NO_ENCODERS);
  }
  // Check VDDIO2 voltage
  int16_t voltage = read_int_voltage(ADC_VDDIO2DIV10);
  if (voltage < 0) {
    Serial.print("Cannot measure VDDIO2 - error code: ");
    Serial.println(voltage_adc + (int32_t)2100000000);
  } else {
    int16_t voltage = voltage_adc;
    uint8_t lsbs = voltage & 3;
    voltage >> 2; // divide by 4. We took a 12 bit reading with a 1.024 V reference. VDDIO2 is expected to be either 3.3V or 5V. Divided by 10 gives 330mv or 500mv.
    // With that reference, we should thus expect 1320 or 2000 to be read. We would like millivolts, so divide by 4 after recording the two low bits.
    // We would like to see 330 or 500 at that point.
    voltage *= 10; // multiply by 10 to undo the internal division by 10 yielding actual mV.
    if (lsbs & 1)
      voltage += 3;
    if (lsbs & 2)
      voltage += 5;
    // So now we have the voltage on vddio2 in millivolts.
    Serial.print("VDDIO2 = ")
    Serial.println(voltage);
  }
  if (PortC_OK) {
    VPORTC.INTFLAGS = VPORTC.INTFLAGS;
    PORTC.PIN0CTRL = PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
    PORTC.PIN1CTRL = PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
    PORTC.PIN2CTRL = PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
    PORTC.PIN3CTRL = PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
    pinModeFast(ENC2_BTN, INPUT_PULLUP);
    pinModeFast(ENC2_BTN, INPUT_PULLUP);
#define MODE_BTN    PIN_PB5
  }


}

int16_t read_int_voltage(uint8_t chan) {
  if (chan != ADC_VDDDIV10 && chan != ADC_VDDIO2DIV10) {
    return -32768; // special error code when bad arguments are passed
  }
  uint8_t oldref = getAnalogReference();
  analogReference(INTERNAL1V024);
  analogRead(chan); // throw away first reading.
  uint32_t voltage_adc = analogReadEnh(chan, 12);
  if (analogIsError(voltage_adc)) {
    analogReference(oldref);
    return ((int16_t)voltage_adc);
  } else {
    int16_t voltage = voltage_adc;
    uint8_t lsbs = voltage & 3;
    voltage >> 2; // divide by 4. We took a 12 bit reading with a 1.024 V reference. VDD should be ~5V. Divided by 10 gives ~500mv.
    // With that reference, we should thus expect 2000 to be read. We would like millivolts, so divide by 4 after recording the two low bits.
    // so we should see
    voltage *= 10; // multiply by 10 to undo the internal division by 10 yielding actual mV.
    if (lsbs & 1) {
      voltage += 3;
    }
    if (lsbs & 2) {
      voltage += 5;
    }
    analogReference(oldref);
    return (voltage);
  }
}

bool init_POST() {

}
bool init_LCD() {

}
