#define  XTC    TC1       // TC number
#define  XCHAN  1         // TC channel
#define  XCHAN2 2         // TC channel2
#define  XID    ID_TC4    // Instance ID
#define  XID2   ID_TC5

void setup(){
  Serial.begin(9600);
  pmc_set_writeprotect(false);
  pmc_enable_periph_clk(XID);
  TC_Configure(XTC, XCHAN, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK4);//一切照旧
  TC_Configure(XTC, XCHAN2, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK2);
}

void loop(){
    double result1, result2;
  TC_Start(XTC, XCHAN);
  TC_Start(XTC, XCHAN2);
  
  fprint(1.2);//channel1计时1个fprint

  result1=(double)TC_ReadCV(XTC, XCHAN);//clock4的frequency为MCK/128，Microsecond是1us

  fprint(1.2);//channel2计时2个fprint

  result2=(double)TC_ReadCV(XTC, XCHAN2)/128*8;
  Serial.println(result1, DEC);
  Serial.println(result2, DEC);
  TC_Stop(XTC, XCHAN);
  TC_Stop(XTC, XCHAN2);
}

void fprint(double f) { //print aouble format data
  Serial.println(f);
}

